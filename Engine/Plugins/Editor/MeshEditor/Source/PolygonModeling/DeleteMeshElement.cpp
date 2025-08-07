// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeleteMeshElement.h"

#include "ContentBrowserModule.h"
#include "EditableMesh.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IContentBrowserSingleton.h"
#include "IMeshEditorModeEditingContract.h"
#include "IMeshEditorModeUIContract.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "MeshEditorMode"

void UDeleteMeshElementCommand::RegisterUICommand( FBindingContext* BindingContext )
{
	UI_COMMAND_EXT( BindingContext, /* Out */ UICommandInfo, "DeleteMeshElement", "Delete", "Delete selected mesh elements, including polygons partly defined by selected elements.", EUserInterfaceActionType::Button, FInputChord( EKeys::Delete ) );
}

void UDeleteMeshElementCommand::Execute( IMeshEditorModeEditingContract& MeshEditorMode )
{
	if( MeshEditorMode.GetActiveAction() != NAME_None )
	{
		return;
	}

	TMap< UEditableMesh*, TArray< FMeshElement > > MeshesWithElementsToDelete;
	MeshEditorMode.GetSelectedMeshesAndElements( EEditableMeshElementType::Any, MeshesWithElementsToDelete );
	if( MeshesWithElementsToDelete.Num() == 0 )
	{
		return;
	}

	FScopedTransaction Transaction( LOCTEXT( "UndoDeleteMeshElement", "Delete" ) );

	MeshEditorMode.CommitSelectedMeshes();

	// Refresh selection (committing may have created a new mesh instance)
	MeshEditorMode.GetSelectedMeshesAndElements( EEditableMeshElementType::Any, MeshesWithElementsToDelete );

	// Deselect the mesh elements before we delete them.  This will make sure they become selected again after undo.
	MeshEditorMode.DeselectMeshElements( MeshesWithElementsToDelete );

	for( const auto& MeshAndElements : MeshesWithElementsToDelete )
	{
		UEditableMesh* EditableMesh = MeshAndElements.Key;

		EditableMesh->StartModification( EMeshModificationType::Final, EMeshTopologyChange::TopologyChange );

		for( const FMeshElement& MeshElementToDelete : MeshAndElements.Value )
		{
			const bool bDeleteOrphanedEdges = true;
			const bool bDeleteOrphanedVertices = true;
			const bool bDeleteOrphanedVertexInstances = true;
			const bool bDeleteEmptySections = false;

			// If we deleted the same polygon on multiple selected instances of the same mesh, the polygon could already have been deleted
			// by the time we get here
			if( MeshElementToDelete.IsElementIDValid( EditableMesh ) )
			{
				if( MeshElementToDelete.ElementAddress.ElementType == EEditableMeshElementType::Vertex )
				{
					EditableMesh->DeleteVertexAndConnectedEdgesAndPolygons(
						FVertexID( MeshElementToDelete.ElementAddress.ElementID ),
						bDeleteOrphanedEdges,
						bDeleteOrphanedVertices,
						bDeleteOrphanedVertexInstances,
						bDeleteEmptySections );

				}
				else if( MeshElementToDelete.ElementAddress.ElementType == EEditableMeshElementType::Edge )
				{
					EditableMesh->DeleteEdgeAndConnectedPolygons(
						FEdgeID( MeshElementToDelete.ElementAddress.ElementID ),
						bDeleteOrphanedEdges,
						bDeleteOrphanedVertices,
						bDeleteOrphanedVertexInstances,
						bDeleteEmptySections );
				}
				else if( MeshElementToDelete.ElementAddress.ElementType == EEditableMeshElementType::Polygon )
				{
					static TArray<FPolygonID> PolygonIDsToDelete;
					PolygonIDsToDelete.Reset();
					PolygonIDsToDelete.Add( FPolygonID( MeshElementToDelete.ElementAddress.ElementID ) );
					EditableMesh->DeletePolygons( 
						PolygonIDsToDelete,
						bDeleteOrphanedEdges,
						bDeleteOrphanedVertices,
						bDeleteOrphanedVertexInstances,
						bDeleteEmptySections );
				}
			}
		}

		EditableMesh->EndModification();

		MeshEditorMode.TrackUndo( EditableMesh, EditableMesh->MakeUndo() );
	}
}

#undef LOCTEXT_NAMESPACE
