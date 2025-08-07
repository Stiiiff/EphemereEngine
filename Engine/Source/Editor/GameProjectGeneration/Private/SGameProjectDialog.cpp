// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGameProjectDialog.h"
#include "Misc/MessageDialog.h"

#include "Styling/CoreStyle.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "EditorStyleSet.h"
#include "GameProjectUtils.h"
#include "SProjectBrowser.h"
#include "SNewProjectWizard.h"
#include "IDocumentation.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Internationalization/BreakIterator.h"
#include "TemplateCategory.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "GameProjectGeneration"


/* SGameProjectDialog interface
*****************************************************************************/

namespace GameProjectDialogDefs
{
	constexpr float Padding = 5;
	constexpr float TextWidth = 420;
	constexpr float TextHeight = 16;

	constexpr float ThumbnailSize = 64;

	constexpr int32 LandingPageIndex = 0;
	constexpr int32 ProjectBrowserPageIndex = 1;
	constexpr int32 ProjectSettingsPageIndex = 2;
	
	static const FText LandingPageTitle = LOCTEXT("ProjectDialog_SelectOrCreateProject", "Select or Create New Project");
	static const FText SelectCategoryTitle = LOCTEXT("ProjectDialog_SelectTemplateCategory", "Select Template Category");
	static const FText ProjectBrowserTitle = LOCTEXT("ProjectDialog_ProjectBrowserTitle", "Open Existing Project");
	static const FText ProjectSettingsTitle = LOCTEXT("ProjectDialog_ProjectSettings", "Project Settings");
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGameProjectDialog::Construct(const FArguments& InArgs, EMode InMode)
{
	DialogMode = InMode;

	NewProjectWizard = SNew(SNewProjectWizard)
		.OnTemplateDoubleClick(this, &SGameProjectDialog::OnTemplateDoubleClick);
	ProjectBrowserPage = SNew(SProjectBrowser)
		.HideOpenButton(true);

	const float UniformPadding = 16.0f;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		.Padding(8)
		[
			SAssignNew(RootWizard, SWizard)
			.ShowBreadcrumbs(false)
			.ShowPageList(true) // TODO: Revert it when the selection clearing is done
			.ShowPageTitle(true)
			.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
			.CancelButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
			.FinishButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
			.ButtonTextStyle(FEditorStyle::Get(), "LargeText")
			.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
			.FinishButtonText(this, &SGameProjectDialog::GetFinishText)
			.FinishButtonToolTip(this, &SGameProjectDialog::GetFinishTooltip)
			.CanFinish(this, &SGameProjectDialog::OnCanFinish)
			.OnFinished(this, &SGameProjectDialog::OnFinishClicked)
			.OnCanceled(this, &SGameProjectDialog::OnCancelClicked)
			.InitialPageIndex(this, &SGameProjectDialog::GetInitialPageIndex)
			.OnGetNextPageIndex(this, &SGameProjectDialog::GetNextPageIndex)

			+ SWizard::Page()
			.Name(GetPageTitle(GameProjectDialogDefs::LandingPageIndex))
			.CanShow(DialogMode != EMode::Open)
			[
				CreateLandingPage()
			]

			+ SWizard::Page()
			.Name(GetPageTitle(GameProjectDialogDefs::ProjectBrowserPageIndex))
			.CanShow(DialogMode != EMode::New)
			[
				ProjectBrowserPage.ToSharedRef()
			]

			+ SWizard::Page()
			.Name(GetPageTitle(GameProjectDialogDefs::ProjectSettingsPageIndex))
			.CanShow(DialogMode != EMode::Open)
			.OnEnter(this, &SGameProjectDialog::OnEnterSettingsPage)
			[
				SAssignNew(ProjectSettingsPage, SBox)
			]
		]
	];
}

void SGameProjectDialog::GetAllTemplateCategories(TArray<TSharedPtr<FTemplateCategory>>& OutCategories)
{
	TArray<TSharedPtr<FTemplateCategory>> AllTemplateCategories;
	//FGameProjectGenerationModule::Get().GetAllTemplateCategories(AllTemplateCategories);

	if (AllTemplateCategories.Num() == 0)
	{
		TSharedPtr<FTemplateCategory> DefaultCategory = MakeShareable(new FTemplateCategory());
		static const FName DefaultCategoryKey("Default");
		DefaultCategory->Key = DefaultCategoryKey;
		DefaultCategory->DisplayName = LOCTEXT("ProjectDialog_DefaultCategoryName", "Blank Project");
		DefaultCategory->Description = LOCTEXT("ProjectDialog_DefaultCategoryDescription", "Create a new blank Unreal project.");
		DefaultCategory->IsMajor = true;
		DefaultCategory->Icon = FEditorStyle::GetBrush("GameProjectDialog.DefaultGameThumbnail");

		AllTemplateCategories.Add(DefaultCategory);
	}

	OutCategories = AllTemplateCategories;
}

TSharedRef<SWidget> SGameProjectDialog::CreateLandingPage()
{
	TArray<TSharedPtr<FTemplateCategory>> AllTemplateCategories;
	SGameProjectDialog::GetAllTemplateCategories(AllTemplateCategories);

	TSharedRef<SWidget> Widget = SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.FillHeight(1)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SOverlay)
					.Visibility(DialogMode != EMode::New ? EVisibility::Visible : EVisibility::Collapsed )
					+ SOverlay::Slot()
					[
						SAssignNew(RecentProjectBrowser, SRecentProjectBrowser)
						.OnSelectionChanged(this, &SGameProjectDialog::OnRecentProjectSelectionChanged)
						.Visibility(this, &SGameProjectDialog::GetRecentProjectBrowserVisibility)
					]
					+ SOverlay::Slot()
					.Padding(8, 0)
					[
						SNew(SVerticalBox)
						.Visibility(this, &SGameProjectDialog::GetNoRecentProjectsLabelVisibility)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 4, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "GameProjectDialog.ProjectNamePathLabels")
							.Text(LOCTEXT("ProjectDialog_Recent", "Recent Projects"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 2, 0, 4)
						[
							SNew(SSeparator)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 8, 0, 4)
						[
							SNew(SRichTextBlock)
							.DecoratorStyleSet(&FEditorStyle::Get())
							.Text(LOCTEXT("ProjectDialog_NoRecentProjects", "No recent projects found. Press <RichTextBlock.BoldHighlight>More</> to browse for projects."))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(8, 12, 8, 0)
					[
						SNew(SButton)
						.Text(LOCTEXT("ProjectDialog_More", "More"))
						.TextStyle(FEditorStyle::Get(), "LargeText")
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.ForegroundColor(FEditorStyle::Get().GetSlateColor("WhiteBrush"))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FCoreStyle::Get().GetMargin("StandardDialog.ContentPadding"))
						.OnClicked(this, &SGameProjectDialog::OnMoreProjectsClicked)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4, 8, 4)
				[
					SNew(SSeparator)
					.Visibility(DialogMode == EMode::Both ? EVisibility::Visible : EVisibility::Collapsed)
				]
				+ SVerticalBox::Slot()
				.Padding(8)
				.AutoHeight()
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "GameProjectDialog.ProjectNamePathLabels")
					.Text(LOCTEXT("ProjectDialog_NewProjectCategories", "Select a template to create a project"))
					.Visibility(DialogMode == EMode::Both ? EVisibility::Visible : EVisibility::Collapsed)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8, 4, 8, 4)
				[
					SAssignNew(NewProjectWizard, SNewProjectWizard)
					.Visibility(EVisibility::Visible)
				]
			]
		]
	];

	RecentProjectBrowser->ClearSelection();
	NewProjectWizard->SetCurrentCategory("Games");

	return Widget;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

/* SGameProjectDialog callbacks
*****************************************************************************/

TSharedRef<ITableRow> SGameProjectDialog::ConstructTile(TSharedPtr<FTemplateCategory> Item, const TSharedRef<STableViewBase>& TableView)
{
	TSharedRef<STableRow<TSharedPtr<FTemplateCategory>>> Row = SNew(STableRow<TSharedPtr<FTemplateCategory>>, TableView)
	.Style(FEditorStyle::Get(), "GameProjectDialog.TemplateListView.TableRow")
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(0)
		[
			SNew(SHorizontalBox)

			// Icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(GameProjectDialogDefs::Padding)
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.WidthOverride(GameProjectDialogDefs::ThumbnailSize)
				.HeightOverride(GameProjectDialogDefs::ThumbnailSize)
				.Padding(0)
				[
					SNew(SImage)
					.Image(Item->Icon)
				]
			]

			+ SHorizontalBox::Slot()
			.Padding(GameProjectDialogDefs::Padding)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				// Name
				+ SVerticalBox::Slot()
				.MaxHeight(GameProjectDialogDefs::TextHeight)
				.Padding(0, 0, 0, GameProjectDialogDefs::Padding)
				[
					SNew(STextBlock)
					.Text(Item->DisplayName)
					.Justification(ETextJustify::Left)
					.TextStyle(FEditorStyle::Get(), "LargeText")
				]

				// Description
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.LineBreakPolicy(FBreakIterator::CreateWordBreakIterator())
					.Justification(ETextJustify::Left)
					.Text(Item->Description)
				]
			]
		]
	];

	Row->SetToolTipText(FText::Format(FText::FromString("{0}\n{1}"), Item->DisplayName, Item->Description));

	return Row;
}

FReply SGameProjectDialog::OnMoreProjectsClicked()
{
	int32 Index = GameProjectDialogDefs::ProjectBrowserPageIndex;
	RootWizard->AdvanceToPage(Index);
	return FReply::Handled();
}

void SGameProjectDialog::OnTemplateDoubleClick() const
{
	RootWizard->AdvanceToPage(GameProjectDialogDefs::ProjectSettingsPageIndex);
}

void SGameProjectDialog::OnRecentProjectSelectionChanged(FString Item)
{
	if (!Item.IsEmpty())
	{
		// TODO: Clear the selection in new project wizard
	}
}

int32 SGameProjectDialog::GetInitialPageIndex() const
{
	if (DialogMode == EMode::Open)
	{
		return GameProjectDialogDefs::ProjectBrowserPageIndex;
	}
	return GameProjectDialogDefs::LandingPageIndex;
}

int32 SGameProjectDialog::GetNextPageIndex(int32 Current) const
{
	int32 CurrentPage = RootWizard->GetCurrentPageIndex();
	if (CurrentPage == GameProjectDialogDefs::LandingPageIndex)
	{
		FString SelectedProjectFile = RecentProjectBrowser->GetSelectedProjectFile();
		if (!SelectedProjectFile.IsEmpty())
		{
			return INDEX_NONE;
		}

		return GameProjectDialogDefs::ProjectSettingsPageIndex;
	}
	else if (CurrentPage == GameProjectDialogDefs::ProjectBrowserPageIndex)
	{
		if (NewProjectWizard->ShouldShowProjectSettingsPage())
		{
			return GameProjectDialogDefs::ProjectSettingsPageIndex;
		}
	}

	return INDEX_NONE;
}

void SGameProjectDialog::OnEnterSettingsPage()
{
	ProjectSettingsPage->SetContent(NewProjectWizard->CreateProjectSettingsPage());
}

bool SGameProjectDialog::OnCanFinish() const
{
	int32 CurrentPage = RootWizard->GetCurrentPageIndex();
	if (CurrentPage == GameProjectDialogDefs::ProjectSettingsPageIndex)
	{
		return NewProjectWizard->CanCreateProject();
	}


	if (CurrentPage == GameProjectDialogDefs::ProjectBrowserPageIndex)
	{
		return !ProjectBrowserPage->GetSelectedProjectFile().IsEmpty();
	}

	if (CurrentPage == GameProjectDialogDefs::LandingPageIndex)
	{
		if (DialogMode == EMode::New)
		{
			return false;
		}
		else
		{
			return !RecentProjectBrowser->GetSelectedProjectFile().IsEmpty();
		}
	}

	return false;
}

void SGameProjectDialog::OnFinishClicked()
{
	int32 CurrentPage = RootWizard->GetCurrentPageIndex();
	if (CurrentPage == GameProjectDialogDefs::LandingPageIndex)
	{
		RecentProjectBrowser->OpenSelectedProject();
	}
	else if (CurrentPage == GameProjectDialogDefs::ProjectBrowserPageIndex)
	{
		ProjectBrowserPage->OpenSelectedProject();
	}
	else
	{
		NewProjectWizard->CreateAndOpenProject();
	}
}

void SGameProjectDialog::OnCancelClicked() const
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	Window->RequestDestroyWindow();
}

FText SGameProjectDialog::GetFinishText() const
{
	int32 CurrentPage = RootWizard->GetCurrentPageIndex();
	if ((DialogMode == EMode::Open || DialogMode == EMode::Both) &&
		(CurrentPage == GameProjectDialogDefs::ProjectBrowserPageIndex ||
		CurrentPage == GameProjectDialogDefs::LandingPageIndex))
	{
		return NSLOCTEXT("ProjectBrowser", "OpenProjectBrowseTitle", "Open Project");
	}

	return NSLOCTEXT("NewProjectWizard", "Create Project", "Create Project");
}

FText SGameProjectDialog::GetFinishTooltip() const
{
	int32 CurrentPage = RootWizard->GetCurrentPageIndex();
	if (CurrentPage == GameProjectDialogDefs::ProjectSettingsPageIndex)
	{
		return NSLOCTEXT("NewProjectWizard", "FinishButtonToolTip", "Creates your new project in the specified location with the specified template and name.");
	}

	return FText::GetEmpty();
}

FText SGameProjectDialog::GetPageTitle(int32 PageIndex) const
{
	switch (PageIndex)
	{
		case GameProjectDialogDefs::LandingPageIndex:
			return DialogMode == EMode::New ? GameProjectDialogDefs::SelectCategoryTitle : GameProjectDialogDefs::LandingPageTitle;
		case GameProjectDialogDefs::ProjectBrowserPageIndex: return GameProjectDialogDefs::ProjectBrowserTitle;
		case GameProjectDialogDefs::ProjectSettingsPageIndex: return GameProjectDialogDefs::ProjectSettingsTitle;
	}

	return FText::GetEmpty();
}

EVisibility SGameProjectDialog::GetRecentProjectBrowserVisibility() const
{
	if (!RecentProjectBrowser.IsValid())
	{
		return EVisibility::Collapsed;
	}

	return RecentProjectBrowser->HasProjects() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SGameProjectDialog::GetNoRecentProjectsLabelVisibility() const
{
	if (!RecentProjectBrowser.IsValid())
	{
		return EVisibility::Collapsed;
	}

	return RecentProjectBrowser->HasProjects() ? EVisibility::Collapsed : EVisibility::Visible;
}

#undef LOCTEXT_NAMESPACE