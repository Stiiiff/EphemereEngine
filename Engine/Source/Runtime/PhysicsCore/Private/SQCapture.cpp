// Copyright Epic Games, Inc. All Rights Reserved.

#include "SQCapture.h"

#include "Chaos/ImplicitObject.h"
#include "Chaos/PBDRigidsEvolutionGBF.h"
#include "PhysicsInterfaceTypesCore.h"

#include "PhysicsPublicCore.h"
#include "PhysTestSerializer.h" 

class FSQCaptureFilterCallback : public ICollisionQueryFilterCallbackBase
{
public:
	FSQCaptureFilterCallback(const FSQCapture& InCapture) : Capture(InCapture) {}
	virtual ~FSQCaptureFilterCallback() {}
	virtual ECollisionQueryHitType PostFilter(const FCollisionFilterData& FilterData, const ChaosInterface::FQueryHit& Hit) override { /*check(false);*/  return ECollisionQueryHitType::Touch; }
	virtual ECollisionQueryHitType PreFilter(const FCollisionFilterData& FilterData, const Chaos::FPerShapeData& Shape, const Chaos::FGeometryParticle& Actor) override { return Capture.GetFilterResult(&Shape, &Actor); }

private:
	const FSQCapture& Capture;
};

FSQCapture::FSQCapture(FPhysTestSerializer& InPhysSerializer)
	: OutputFlags(EHitFlags::None)
	, ChaosGeometry(nullptr)
	, PhysSerializer(InPhysSerializer)
	, bDiskDataIsChaos(false)
	, bChaosDataReady(false)
	, bPhysXDataReady(false)
{
}

FSQCapture::~FSQCapture()
{
}

void SerializeQueryFilterData(FArchive& Ar, FQueryFilterData& QueryFilterData)
{
	Ar << QueryFilterData.data.word0;
	Ar << QueryFilterData.data.word1;
	Ar << QueryFilterData.data.word2;
	Ar << QueryFilterData.data.word3;
	uint16 Flags = QueryFilterData.flags;
	Ar << Flags;
	QueryFilterData.flags = (FChaosQueryFlags)Flags;
	Ar << QueryFilterData.clientId;
}

void FSQCapture::SerializeChaosActorToShapeHitsArray(Chaos::FChaosArchive& Ar)
{
	int32 NumActors = ChaosActorToShapeHitsArray.Num();
	Ar << NumActors;
	if (Ar.IsLoading())
	{
		for (int32 ActorIdx = 0; ActorIdx < NumActors; ++ActorIdx)
		{
			Chaos::TSerializablePtr<Chaos::FGeometryParticle>Actor;
			Ar << Actor;
			int32 NumShapes;
			Ar << NumShapes;

			TArray<TPair<Chaos::FPerShapeData*, ECollisionQueryHitType>> Pairs;
			for (int32 ShapeIdx = 0; ShapeIdx < NumShapes; ++ShapeIdx)
			{
				Chaos::TSerializablePtr<Chaos::FPerShapeData> Shape;
				Ar << Shape;
				ECollisionQueryHitType HitType;
				Ar << HitType;
				Pairs.Emplace(const_cast<Chaos::FPerShapeData*>(Shape.Get()), HitType);
			}
			ChaosActorToShapeHitsArray.Add(const_cast<Chaos::FGeometryParticle*>(Actor.Get()), Pairs);
		}
	}
	else if (Ar.IsSaving())
	{
		for (auto& Itr : ChaosActorToShapeHitsArray)
		{
			Ar << AsAlwaysSerializable(Itr.Key);
			int32 NumShapes = Itr.Value.Num();
			Ar << NumShapes;

			for (auto& Pair : Itr.Value)
			{
				Ar << AsAlwaysSerializable(Pair.Key);
				Ar << Pair.Value;

			}
		}
	}
}

template <typename THit>
void FSQCapture::SerializeChaosBuffers(Chaos::FChaosArchive& Ar, int32 Version, ChaosInterface::FSQHitBuffer<THit>& ChaosBuffer)
{
	bool bHasBlock = ChaosBuffer.HasBlockingHit();
	Ar << bHasBlock;

	if (bHasBlock)
	{
		if (Ar.IsLoading())
		{
			THit Hit;
			Ar << Hit;
			ChaosBuffer.SetBlockingHit(Hit);
		}
		else
		{
			THit Hit = *ChaosBuffer.GetBlock();
			Ar << Hit;
		}
	}

	int32 NumHits;
	
	NumHits = ChaosBuffer.GetNumHits();
	Ar << NumHits;

	for (int32 Idx = 0; Idx < NumHits; ++Idx)
	{
		if (Ar.IsLoading())
		{
			THit Touch;
			Ar << Touch;
			ChaosBuffer.AddTouchingHit(Touch);
		}
		else
		{
			THit* Hits = ChaosBuffer.GetHits();
			Ar << Hits[Idx];
		}
	}
	
}

void FSQCapture::Serialize(Chaos::FChaosArchive& Ar)
{
	static const FName SQCaptureName = TEXT("SQCapture");
	Chaos::FChaosArchiveScopedMemory ScopedMemory(Ar, SQCaptureName, false);

	int32 Version = 2;
	Ar << Version;
	Ar << SQType;
	Ar << bDiskDataIsChaos;
	Ar << Dir << StartTM << DeltaMag << OutputFlags;
	Ar << GeomData;
	Ar << HitData;

	if (Version >= 1)
	{
		Ar << StartPoint;
	}

#if WITH_PHYSX
	if (bDiskDataIsChaos == false)
	{
		if(Version >= 1)
		{
			SerializeQueryFilterData(Ar, QueryFilterData);
		}
	}
#endif

#if WITH_CHAOS
	if (bDiskDataIsChaos)
	{
#if WITH_CHAOS
		SerializeChaosBuffers(Ar, Version, ChaosSweepBuffer);
		SerializeChaosBuffers(Ar, Version, ChaosRaycastBuffer);
		SerializeChaosBuffers(Ar, Version, ChaosOverlapBuffer);
#endif // WITH_CHAOS

		SerializeChaosActorToShapeHitsArray(Ar);
		SerializeQueryFilterData(Ar, QueryFilterData);

		if (Version >= 2)
		{
			Ar << SerializableChaosGeometry;
			ChaosGeometry = SerializableChaosGeometry.Get();
		}
	}
#endif

	if (Ar.IsLoading())
	{
#if 0
		CreateChaosDataFromPhysX();
#endif // WITH_PHYSX

		if (bDiskDataIsChaos)
		{
			FilterCallback = MakeUnique<FSQCaptureFilterCallback>(*this);
		}
	}
}

void FSQCapture::StartCaptureChaosSweep(const Chaos::FPBDRigidsEvolution& Evolution, const Chaos::FImplicitObject& InQueryGeom, const FTransform& InStartTM, const FVector& InDir, float InDeltaMag, FHitFlags InOutputFlags, const FQueryFilterData& QueryFilter, const FCollisionFilterData& FilterData, ICollisionQueryFilterCallbackBase& Callback)
{
	if (IsInGameThread())
	{
		bDiskDataIsChaos = true;
		CaptureChaosFilterResults(Evolution, FilterData, Callback);
		//copy data
		SerializableChaosGeometry = InQueryGeom.Copy();
		ChaosGeometry = SerializableChaosGeometry.Get();
		StartTM = InStartTM;
		Dir = InDir;
		DeltaMag = InDeltaMag;
		OutputFlags = InOutputFlags;
		QueryFilterData = QueryFilter;

		SQType = ESQType::Sweep;
	}
}

void FSQCapture::EndCaptureChaosSweep(const ChaosInterface::FSQHitBuffer<ChaosInterface::FSweepHit>& Results)
{
#if WITH_CHAOS
	if (IsInGameThread())
	{
		check(SQType == ESQType::Sweep);
#if WITH_CHAOS
		ChaosSweepBuffer = Results;
#endif // WITH_CHAOS
	}
#endif
}

void FSQCapture::StartCaptureChaosRaycast(const Chaos::FPBDRigidsEvolution& Evolution, const FVector& InStartPoint, const FVector& InDir, float InDeltaMag, FHitFlags InOutputFlags, const FQueryFilterData& QueryFilter, const FCollisionFilterData& FilterData, ICollisionQueryFilterCallbackBase& Callback)
{
	if (IsInGameThread())
	{
		bDiskDataIsChaos = true;
		CaptureChaosFilterResults(Evolution, FilterData, Callback);
		//copy data
		StartPoint = InStartPoint;
		Dir = InDir;
		DeltaMag = InDeltaMag;
		OutputFlags = InOutputFlags;
		QueryFilterData = QueryFilter;

		SQType = ESQType::Raycast;
	}
}

void FSQCapture::EndCaptureChaosRaycast(const ChaosInterface::FSQHitBuffer<ChaosInterface::FRaycastHit>& Results)
{
#if WITH_CHAOS
	if (IsInGameThread())
	{
		check(SQType == ESQType::Raycast);
#if WITH_CHAOS
		ChaosRaycastBuffer = Results;
#endif // WITH_CHAOS
	}
#endif
}

void FSQCapture::StartCaptureChaosOverlap(const Chaos::FPBDRigidsEvolution& Evolution, const Chaos::FImplicitObject& InQueryGeom, const FTransform& InStartTM, const FQueryFilterData& QueryFilter, const FCollisionFilterData& FilterData, ICollisionQueryFilterCallbackBase& Callback)
{
	if (IsInGameThread())
	{
		bDiskDataIsChaos = true;
		CaptureChaosFilterResults(Evolution, FilterData, Callback);
		//copy data
		SerializableChaosGeometry = InQueryGeom.Copy();
		ChaosGeometry = SerializableChaosGeometry.Get();
		StartTM = InStartTM;
		QueryFilterData = QueryFilter;

		SQType = ESQType::Overlap;
	}
}

void FSQCapture::EndCaptureChaosOverlap(const ChaosInterface::FSQHitBuffer<ChaosInterface::FOverlapHit>& Results)
{
#if WITH_CHAOS
	if (IsInGameThread())
	{
		check(SQType == ESQType::Overlap);
#if WITH_CHAOS
		ChaosOverlapBuffer = Results;
#endif // WITH_CHAOS
	}
#endif
}

void FSQCapture::CaptureChaosFilterResults(const Chaos::FPBDRigidsEvolution& TransientEvolution, const FCollisionFilterData& FilterData, ICollisionQueryFilterCallbackBase& Callback)
{
	using namespace Chaos;
	const FPBDRigidsSOAs& Particles = TransientEvolution.GetParticles();
	const int32 NumTransientActors = Particles.GetParticleHandles().Size();

	for (int32 Idx = 0; Idx < NumTransientActors; ++Idx)
	{
		FGeometryParticle* TransientActor = Particles.GetParticleHandles().Handle(Idx)->GTGeometryParticle();
		const FShapesArray& TransientShapes = TransientActor->ShapesArray();
		const int32 NumTransientShapes = TransientShapes.Num();
		TArray<TPair<FPerShapeData*, ECollisionQueryHitType>> ShapeHitsArray; ShapeHitsArray.Reserve(NumTransientShapes);
		for (const auto& TransientShape : TransientShapes)
		{
			const ECollisionQueryHitType Result = Callback.PreFilter(FilterData, *TransientShape, *TransientActor);
			ShapeHitsArray.Emplace(TransientShape.Get(), Result);
		}

		ChaosActorToShapeHitsArray.Add(TransientActor, ShapeHitsArray);
	}
}

template <typename TShape, typename TActor>
ECollisionQueryHitType GetFilterResultHelper(const TShape* Shape, const TActor* Actor, const TMap<TActor*, TArray<TPair<TShape*, ECollisionQueryHitType>>>& ActorToShapeHitsArray)
{
	if (const TArray<TPair<TShape*, ECollisionQueryHitType>>* ActorToPairs = ActorToShapeHitsArray.Find(Actor))
	{
		for (const TPair<TShape*, ECollisionQueryHitType>& Pair : *ActorToPairs)
		{
			if (Pair.Key == Shape)
			{
				return Pair.Value;
			}
		}
	}

	//todo: figure out why this hits - suspect it's related to threading and how we capture an evolution on GT
	ensure(false);	//should not get here, means we didn't properly capture all filter results
	return ECollisionQueryHitType::None;
}

ECollisionQueryHitType FSQCapture::GetFilterResult(const Chaos::FPerShapeData* Shape, const Chaos::FGeometryParticle* Actor) const
{
	return GetFilterResultHelper(Shape, Actor, ChaosActorToShapeHitsArray);
}

#if 0
void PhysXQueryHitToChaosQueryHit(ChaosInterface::FQueryHit& ChaosHit, const PxQueryHit& PxHit, const FPhysTestSerializer& Serializer)
{
	ChaosHit.Actor = Serializer.PhysXActorToChaosHandle(PxHit.actor);
	ChaosHit.Shape = Serializer.PhysXShapeToChaosImplicit(PxHit.shape);
}

void PhysXLocationHitToChaosLocationHit(ChaosInterface::FLocationHit& ChaosHit, const PxLocationHit& PxHit, const FPhysTestSerializer& Serializer)
{
	PhysXQueryHitToChaosQueryHit(ChaosHit, PxHit, Serializer);
	ChaosHit.WorldPosition = P2UVector(PxHit.position);
	ChaosHit.WorldNormal = P2UVector(PxHit.normal);
	ChaosHit.Flags = P2UHitFlags(PxHit.flags);
	ChaosHit.Distance = PxHit.distance;
}

void PhysXHitToChaosHit(ChaosInterface::FSweepHit& ChaosHit, const PxSweepHit& PxHit, const FPhysTestSerializer& Serializer)
{
	PhysXLocationHitToChaosLocationHit(ChaosHit, PxHit, Serializer);
}

void PhysXHitToChaosHit(ChaosInterface::FOverlapHit& ChaosHit, const PxOverlapHit& PxHit, const FPhysTestSerializer& Serializer)
{
	PhysXQueryHitToChaosQueryHit(ChaosHit, PxHit, Serializer);
}

void PhysXHitToChaosHit(ChaosInterface::FRaycastHit& ChaosHit, const PxRaycastHit& PxHit, const FPhysTestSerializer& Serializer)
{
	PhysXLocationHitToChaosLocationHit(ChaosHit, PxHit, Serializer);
	ChaosHit.U = PxHit.u;
	ChaosHit.V = PxHit.v;
}

template <typename TChaosHit, typename TPhysXHit>
void PhysXToChaosBufferData(ChaosInterface::FSQHitBuffer<TChaosHit>& ChaosBuffer, const PhysXInterface::FDynamicHitBuffer<TPhysXHit>& PhysXBuffer, const FPhysTestSerializer& PhysSerializer)
{
	if (PhysXBuffer.hasBlock)
	{
		TChaosHit Hit;
		PhysXHitToChaosHit(Hit, PhysXBuffer.block, PhysSerializer);
		ChaosBuffer.SetBlockingHit(Hit);

	}
	const int32 NumHits = PhysXBuffer.GetNumHits();
	for (int32 Idx = 0; Idx < NumHits; ++Idx)
	{
		TChaosHit Hit;
		PhysXHitToChaosHit(Hit, PhysXBuffer.GetHits()[Idx], PhysSerializer);
		ChaosBuffer.AddTouchingHit(Hit);
	}
}

void FSQCapture::CreateChaosFilterResults()
{
#if WITH_PHYSX
	for (auto Itr : PxActorToShapeHitsArray)
	{
		Chaos::FGeometryParticle* Actor = PhysSerializer.PhysXActorToChaosHandle(Itr.Key);
		const auto& ShapesArray = Actor->ShapesArray();
		TArray<TPair<Chaos::FPerShapeData*, ECollisionQueryHitType>> FilterResults;

		const auto& Pairs = Itr.Value;
		int32 Idx = 0;
		for (const auto& Pair : Pairs)
		{
			FilterResults.Add(TPair<Chaos::FPerShapeData*, ECollisionQueryHitType>(ShapesArray[Idx++].Get(), Pair.Value));
		}
		ChaosActorToShapeHitsArray.Add(Actor, FilterResults);
	}
#endif
}

void FSQCapture::CreateChaosDataFromPhysX()
{
	using namespace Chaos;
	
	if (bDiskDataIsChaos || bChaosDataReady)
	{
		return;
	}

	if (AlignedDataHelper && AlignedDataHelper->Shape)
	{
		TUniquePtr<TImplicitObjectTransformed<float, 3>> TransformedObj = PxShapeToChaosGeom(AlignedDataHelper->Shape);
		//we know the dummy px shape has no transform so we want to use the inner object
		ChaosGeometry = TransformedObj->GetTransformedObject();
		ChaosOwnerObject = MoveTemp(TransformedObj);
	}

#if WITH_CHAOS
	PhysXToChaosBufferData(ChaosRaycastBuffer, PhysXRaycastBuffer, PhysSerializer);
	PhysXToChaosBufferData(ChaosSweepBuffer, PhysXSweepBuffer, PhysSerializer);
	PhysXToChaosBufferData(ChaosOverlapBuffer, PhysXOverlapBuffer, PhysSerializer);
#endif // WITH_CHAOS

	CreateChaosFilterResults();
	bChaosDataReady = true;
}
#endif
