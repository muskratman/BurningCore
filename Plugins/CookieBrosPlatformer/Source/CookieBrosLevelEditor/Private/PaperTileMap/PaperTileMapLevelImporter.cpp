#include "PaperTileMap/PaperTileMapLevelImporter.h"

#include "Editor.h"
#include "FileHelpers.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PaperTileLayer.h"
#include "PaperTileMap.h"
#include "PaperTileSet.h"
#include "Platformer/Environment/PlatformerBlock.h"
#include "Platformer/Environment/PlatformerConveyor.h"
#include "Platformer/Environment/PlatformerDangerBlock.h"
#include "Platformer/Environment/PlatformerGravityVolume.h"
#include "Platformer/Environment/PlatformerLadder.h"
#include "Platformer/Environment/PlatformerLightningBase.h"
#include "Platformer/Environment/PlatformerMovingPlatform.h"
#include "Platformer/Environment/PlatformerRamp.h"
#include "Platformer/Environment/PlatformerSlipperyBlock.h"
#include "Platformer/Environment/PlatformerSoftPlatform.h"
#include "Platformer/Environment/PlatformerSpikes.h"
#include "Platformer/Environment/PlatformerStream.h"
#include "Platformer/Environment/PlatformerSwitch.h"
#include "Platformer/Environment/PlatformerTriggeredLift.h"
#include "TileMap/TileSetAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogCookieBrosPaperTileMapImport, Log, All);

namespace CookieBrosPaperTileMapImport
{
	static const FName GeneratedActorTag(TEXT("CookieBros.GeneratedFromPaperTileMap"));
	static const FString ImportTileMapArg(TEXT("CookieBrosImportPaperTileMap="));
	static const FString ImportMappingArg(TEXT("CookieBrosImportTileSet="));
	static const FString ImportGameModeArg(TEXT("CookieBrosImportGameMode="));
	static const FString OutputLevelArg(TEXT("CookieBrosImportOutputLevel="));
	static const FString GeneratedLevelsRoot(TEXT("/Game/PlatformerLevels"));
	static constexpr float ImportedBlockDepth = 500.0f;

	enum class EImportedActorKind : uint8
	{
		Block,
		SoftPlatform,
		Spikes,
		Ramp,
		Ladder,
		PlayerStart,
		MovingPlatform,
		TriggeredLift,
		Stream,
		GravityVolume,
		Switch,
		GenericActor
	};

	struct FImportArgs
	{
		FString SourceTileMapObjectPath;
		FString ImportMappingObjectPath;
		FString GameModeClassObjectPath;
		FString OutputLevelPackagePath;
		FPaperTileMapGenerationSettings GenerationSettings;
	};

	struct FResolvedTileCell
	{
		struct FBlockTileDescriptor
		{
			EPlatformerBlockMeshVariant MeshVariant = EPlatformerBlockMeshVariant::FullSize;
		};

		struct FRampTileDescriptor
		{
			bool bIsRamp = false;
			int32 AngleDegrees = 45;
			int32 SegmentIndex = 0;
		};

		FPaperTileInfo TileInfo;
		FName UserDataName = NAME_None;
		int32 SourceLayerIndex = INDEX_NONE;
		FBlockTileDescriptor BlockDescriptor;
		FRampTileDescriptor RampDescriptor;
	};

	struct FSpawnRecipe
	{
		TSubclassOf<AActor> ActorClass;
		EImportedActorKind ActorKind = EImportedActorKind::Block;
		FName DebugName = NAME_None;
	};

	struct FPreparedSpawn
	{
		int32 TileX = INDEX_NONE;
		int32 TileY = INDEX_NONE;
		int32 TileSpanX = 1;
		int32 TileSpanY = 1;
		EPlatformerBlockMeshVariant BlockMeshVariant = EPlatformerBlockMeshVariant::FullSize;
		int32 RampAngleDegrees = INDEX_NONE;
		FResolvedTileCell ResolvedCell;
		FSpawnRecipe SpawnRecipe;
		FTransform LocalSpawnTransform;
	};

	FString NormalizeObjectPath(const FString& InPath)
	{
		const FString TrimmedPath = InPath.TrimStartAndEnd();
		if (TrimmedPath.IsEmpty())
		{
			return FString();
		}

		if (TrimmedPath.Contains(TEXT(".")))
		{
			return TrimmedPath;
		}

		const FString AssetName = FPackageName::GetLongPackageAssetName(TrimmedPath);
		return FString::Printf(TEXT("%s.%s"), *TrimmedPath, *AssetName);
	}

	FString NormalizePackagePath(const FString& InPath)
	{
		const FString TrimmedPath = InPath.TrimStartAndEnd();
		if (TrimmedPath.IsEmpty())
		{
			return FString();
		}

		return TrimmedPath.Contains(TEXT("."))
			? FPackageName::ObjectPathToPackageName(TrimmedPath)
			: TrimmedPath;
	}

	FString BuildDefaultOutputLevelPackagePath(const FString& SourceTileMapObjectPath)
	{
		const FString SourcePackagePath = FPackageName::ObjectPathToPackageName(SourceTileMapObjectPath);
		const FString TileMapAssetName = FPackageName::GetLongPackageAssetName(SourcePackagePath);
		return FString::Printf(TEXT("%s/%s"), *GeneratedLevelsRoot, *TileMapAssetName);
	}

	bool TryParseCommandLine(FImportArgs& OutArgs, FString& OutError)
	{
		FString SourceTileMapArgValue;
		if (!FParse::Value(FCommandLine::Get(), *ImportTileMapArg, SourceTileMapArgValue))
		{
			OutError = TEXT("Missing -CookieBrosImportPaperTileMap=<ObjectPath> command line argument.");
			return false;
		}

		OutArgs.SourceTileMapObjectPath = NormalizeObjectPath(SourceTileMapArgValue);
		if (!FPackageName::IsValidObjectPath(OutArgs.SourceTileMapObjectPath))
		{
			OutError = FString::Printf(TEXT("Invalid PaperTileMap object path: %s"), *OutArgs.SourceTileMapObjectPath);
			return false;
		}

		FString ImportMappingArgValue;
		if (FParse::Value(FCommandLine::Get(), *ImportMappingArg, ImportMappingArgValue))
		{
			OutArgs.ImportMappingObjectPath = NormalizeObjectPath(ImportMappingArgValue);
			if (!FPackageName::IsValidObjectPath(OutArgs.ImportMappingObjectPath))
			{
				OutError = FString::Printf(TEXT("Invalid import mapping asset object path: %s"), *OutArgs.ImportMappingObjectPath);
				return false;
			}
		}

		FString ImportGameModeArgValue;
		if (FParse::Value(FCommandLine::Get(), *ImportGameModeArg, ImportGameModeArgValue))
		{
			OutArgs.GameModeClassObjectPath = NormalizeObjectPath(ImportGameModeArgValue);
		}

		FString OutputLevelArgValue;
		if (!FParse::Value(FCommandLine::Get(), *OutputLevelArg, OutputLevelArgValue))
		{
			OutArgs.OutputLevelPackagePath = BuildDefaultOutputLevelPackagePath(OutArgs.SourceTileMapObjectPath);
		}
		else
		{
			OutArgs.OutputLevelPackagePath = NormalizePackagePath(OutputLevelArgValue);
		}

		if (!FPackageName::IsValidLongPackageName(OutArgs.OutputLevelPackagePath))
		{
			OutError = FString::Printf(TEXT("Invalid output level path: %s"), *OutArgs.OutputLevelPackagePath);
			return false;
		}

		return true;
	}

	UPaperTileMap* LoadTileMapAsset(const FString& ObjectPath)
	{
		return LoadObject<UPaperTileMap>(nullptr, *ObjectPath);
	}

	UTileSetAsset* LoadImportMappingAsset(const FString& ObjectPath)
	{
		return LoadObject<UTileSetAsset>(nullptr, *ObjectPath);
	}

	TSubclassOf<AGameModeBase> LoadGameModeClass(const FString& ObjectPath)
	{
		return ObjectPath.IsEmpty() ? nullptr : LoadClass<AGameModeBase>(nullptr, *ObjectPath);
	}

	FString GetTileMapObjectPath(const UPaperTileMap& TileMap)
	{
		return TileMap.GetPathName();
	}

	EImportedActorKind DetermineActorKind(const UClass* ActorClass)
	{
		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlayerStart::StaticClass()))
		{
			return EImportedActorKind::PlayerStart;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerLadder::StaticClass()))
		{
			return EImportedActorKind::Ladder;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerSoftPlatform::StaticClass()))
		{
			return EImportedActorKind::SoftPlatform;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerSpikes::StaticClass()))
		{
			return EImportedActorKind::Spikes;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerRamp::StaticClass()))
		{
			return EImportedActorKind::Ramp;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerTriggeredLift::StaticClass()))
		{
			return EImportedActorKind::TriggeredLift;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerMovingPlatform::StaticClass()))
		{
			return EImportedActorKind::MovingPlatform;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerStream::StaticClass()))
		{
			return EImportedActorKind::Stream;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerGravityVolume::StaticClass()))
		{
			return EImportedActorKind::GravityVolume;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerSwitch::StaticClass()))
		{
			return EImportedActorKind::Switch;
		}

		if ((ActorClass != nullptr) && ActorClass->IsChildOf(APlatformerBlock::StaticClass()))
		{
			return EImportedActorKind::Block;
		}

		return EImportedActorKind::GenericActor;
	}

	bool TryResolveImportMappingSpawnRecipe(const UTileSetAsset* ImportMappingAsset, FName UserDataName, FSpawnRecipe& OutRecipe)
	{
		if (ImportMappingAsset == nullptr)
		{
			return false;
		}

		const FPaperTileImportRule* ImportRule = ImportMappingAsset->FindRuleByUserDataName(UserDataName);
		if (ImportRule == nullptr)
		{
			return false;
		}

		if (!ImportRule->ImportActorClass)
		{
			UE_LOG(
				LogCookieBrosPaperTileMapImport,
				Warning,
				TEXT("Import mapping asset '%s' matched UserDataName '%s', but ImportActorClass is empty."),
				*ImportMappingAsset->GetPathName(),
				*UserDataName.ToString());
			return false;
		}

		OutRecipe.ActorClass = ImportRule->ImportActorClass;
		OutRecipe.ActorKind = DetermineActorKind(ImportRule->ImportActorClass.Get());
		OutRecipe.DebugName = UserDataName.IsNone() ? FName(TEXT("ImportRule")) : UserDataName;
		return true;
	}

	bool TryParseRampTileDescriptor(FName UserDataName, FResolvedTileCell::FRampTileDescriptor& OutDescriptor)
	{
		FString DescriptorString = UserDataName.ToString().TrimStartAndEnd();
		if (DescriptorString.IsEmpty())
		{
			return false;
		}

		DescriptorString.ReplaceInline(TEXT("-"), TEXT("_"));

		TArray<FString> Tokens;
		DescriptorString.ParseIntoArray(Tokens, TEXT("_"), true);
		if ((Tokens.Num() != 3) || !Tokens[0].Equals(TEXT("Ramp"), ESearchCase::IgnoreCase))
		{
			return false;
		}

		int32 ParsedAngle = 0;
		int32 ParsedSegmentIndex = 0;
		if (!LexTryParseString(ParsedAngle, *Tokens[1]) || !LexTryParseString(ParsedSegmentIndex, *Tokens[2]))
		{
			return false;
		}

		OutDescriptor.bIsRamp = true;
		OutDescriptor.AngleDegrees = FMath::Max(1, ParsedAngle);
		OutDescriptor.SegmentIndex = FMath::Max(0, ParsedSegmentIndex);
		return true;
	}

	bool IsHalfBlockUserDataName(FName UserDataName)
	{
		return UserDataName.ToString().Equals(TEXT("HalfBlock"), ESearchCase::IgnoreCase);
	}

	bool TryGetResolvedTileCell(const UPaperTileMap& TileMap, int32 TileX, int32 TileY, FResolvedTileCell& OutCell)
	{
		for (int32 LayerIndex = TileMap.TileLayers.Num() - 1; LayerIndex >= 0; --LayerIndex)
		{
			const UPaperTileLayer* Layer = TileMap.TileLayers[LayerIndex];
			if ((Layer == nullptr) || !Layer->ShouldRenderInGame())
			{
				continue;
			}

			const FPaperTileInfo TileInfo = Layer->GetCell(TileX, TileY);
			if (!TileInfo.IsValid() || (TileInfo.TileSet == nullptr))
			{
				continue;
			}

			OutCell.TileInfo = TileInfo;
			OutCell.UserDataName = TileInfo.TileSet->GetTileUserData(TileInfo.GetTileIndex());
			OutCell.SourceLayerIndex = LayerIndex;
			OutCell.BlockDescriptor.MeshVariant = IsHalfBlockUserDataName(OutCell.UserDataName)
				? EPlatformerBlockMeshVariant::HalfSize
				: EPlatformerBlockMeshVariant::FullSize;
			TryParseRampTileDescriptor(OutCell.UserDataName, OutCell.RampDescriptor);
			return true;
		}

		return false;
	}

	FTransform BuildTilePermutationTransform(const FPaperTileInfo& TileInfo)
	{
		return UPaperTileLayer::GetTileTransform(TileInfo.GetFlagsAsIndex());
	}

	FVector SanitizeBlockSize(const FVector& InBlockSize)
	{
		return InBlockSize.ComponentMax(FVector(1.0f, 1.0f, 1.0f));
	}

	FVector BuildSolidActorSize(const FVector& BlockSize, int32 TileSpanX = 1)
	{
		return FVector(BlockSize.X * FMath::Max(TileSpanX, 1), ImportedBlockDepth, BlockSize.Z);
	}

	FVector BuildRectActorSize(const FVector& BlockSize, int32 TileSpanX = 1, int32 TileSpanY = 1)
	{
		return FVector(
			BlockSize.X * FMath::Max(TileSpanX, 1),
			BlockSize.Y,
			BlockSize.Z * FMath::Max(TileSpanY, 1));
	}

	FVector BuildDeepRectActorSize(const FVector& BlockSize, int32 TileSpanX = 1, int32 TileSpanY = 1)
	{
		return FVector(
			BlockSize.X * FMath::Max(TileSpanX, 1),
			ImportedBlockDepth,
			BlockSize.Z * FMath::Max(TileSpanY, 1));
	}

	bool IsPlainMergeableBlockUserDataName(FName UserDataName)
	{
		return UserDataName.ToString().Equals(TEXT("Block"), ESearchCase::IgnoreCase)
			|| UserDataName.ToString().Equals(TEXT("HalfBlock"), ESearchCase::IgnoreCase);
	}

	bool IsHorizontalMergeCandidate(const FPreparedSpawn& PreparedSpawn)
	{
		const UClass* ActorClass = PreparedSpawn.SpawnRecipe.ActorClass.Get();
		if (ActorClass == nullptr)
		{
			return false;
		}

		if (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::MovingPlatform
			|| PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::TriggeredLift
			|| PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::SoftPlatform
			|| PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::Spikes)
		{
			return true;
		}

		if (PreparedSpawn.SpawnRecipe.ActorKind != EImportedActorKind::Block)
		{
			return false;
		}

		if (IsPlainMergeableBlockUserDataName(PreparedSpawn.ResolvedCell.UserDataName))
		{
			return true;
		}

		return ActorClass->IsChildOf(APlatformerSlipperyBlock::StaticClass())
			|| ActorClass->IsChildOf(APlatformerDangerBlock::StaticClass())
			|| ActorClass->IsChildOf(APlatformerConveyor::StaticClass());
	}

	bool IsVerticalMergeCandidate(const FPreparedSpawn& PreparedSpawn)
	{
		return PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::Ladder;
	}

	bool IsRectangleMergeCandidate(const FPreparedSpawn& PreparedSpawn)
	{
		return PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::Stream
			|| PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::GravityVolume;
	}

	bool ArePreparedSpawnsMergeCompatible(const FPreparedSpawn& Left, const FPreparedSpawn& Right)
	{
		return (Left.SpawnRecipe.ActorClass == Right.SpawnRecipe.ActorClass)
			&& (Left.SpawnRecipe.ActorKind == Right.SpawnRecipe.ActorKind)
			&& (Left.BlockMeshVariant == Right.BlockMeshVariant)
			&& (Left.RampAngleDegrees == Right.RampAngleDegrees)
			&& (Left.ResolvedCell.SourceLayerIndex == Right.ResolvedCell.SourceLayerIndex)
			&& Left.LocalSpawnTransform.GetRotation().Equals(Right.LocalSpawnTransform.GetRotation(), KINDA_SMALL_NUMBER)
			&& Left.LocalSpawnTransform.GetScale3D().Equals(Right.LocalSpawnTransform.GetScale3D(), KINDA_SMALL_NUMBER);
	}

	bool CanMergeHorizontally(const FPreparedSpawn& CurrentRun, const FPreparedSpawn& Candidate)
	{
		return IsHorizontalMergeCandidate(CurrentRun)
			&& IsHorizontalMergeCandidate(Candidate)
			&& (CurrentRun.TileY == Candidate.TileY)
			&& ArePreparedSpawnsMergeCompatible(CurrentRun, Candidate);
	}

	FVector BuildMergedCenterLocation(const FPreparedSpawn& StartSpawn, const FPreparedSpawn& EndSpawn)
	{
		return (StartSpawn.LocalSpawnTransform.GetLocation() + EndSpawn.LocalSpawnTransform.GetLocation()) * 0.5f;
	}

	FVector BuildBottomAnchoredMergedLocation(const FPreparedSpawn& StartSpawn, const FPreparedSpawn& EndSpawn)
	{
		const FVector StartLocation = StartSpawn.LocalSpawnTransform.GetLocation();
		const FVector EndLocation = EndSpawn.LocalSpawnTransform.GetLocation();

		return FVector(
			(StartLocation.X + EndLocation.X) * 0.5f,
			(StartLocation.Y + EndLocation.Y) * 0.5f,
			FMath::Min(StartLocation.Z, EndLocation.Z));
	}

	TArray<FPreparedSpawn> BuildMergedPreparedSpawns(const TArray<FPreparedSpawn>& PreparedSpawns)
	{
		TArray<FPreparedSpawn> MergedPreparedSpawns;
		MergedPreparedSpawns.Reserve(PreparedSpawns.Num());

		TMap<FIntPoint, int32> SpawnIndexByCell;
		SpawnIndexByCell.Reserve(PreparedSpawns.Num());
		for (int32 SpawnIndex = 0; SpawnIndex < PreparedSpawns.Num(); ++SpawnIndex)
		{
			SpawnIndexByCell.Add(FIntPoint(PreparedSpawns[SpawnIndex].TileX, PreparedSpawns[SpawnIndex].TileY), SpawnIndex);
		}

		TBitArray<> bConsumed(false, PreparedSpawns.Num());

		for (int32 SpawnIndex = 0; SpawnIndex < PreparedSpawns.Num(); ++SpawnIndex)
		{
			if (bConsumed[SpawnIndex])
			{
				continue;
			}

			FPreparedSpawn CurrentRun = PreparedSpawns[SpawnIndex];
			bConsumed[SpawnIndex] = true;

			if (IsRectangleMergeCandidate(CurrentRun))
			{
				int32 RectWidth = 1;
				while (true)
				{
					const int32 NextTileX = CurrentRun.TileX + RectWidth;
					const int32* NextIndex = SpawnIndexByCell.Find(FIntPoint(NextTileX, CurrentRun.TileY));
					if ((NextIndex == nullptr) || bConsumed[*NextIndex] || !ArePreparedSpawnsMergeCompatible(CurrentRun, PreparedSpawns[*NextIndex]))
					{
						break;
					}

					++RectWidth;
				}

				int32 RectHeight = 1;
				while (true)
				{
					const int32 NextTileY = CurrentRun.TileY + RectHeight;
					bool bCanExtendRow = true;

					for (int32 OffsetX = 0; OffsetX < RectWidth; ++OffsetX)
					{
						const int32* CandidateIndex = SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX + OffsetX, NextTileY));
						if ((CandidateIndex == nullptr) || bConsumed[*CandidateIndex] || !ArePreparedSpawnsMergeCompatible(CurrentRun, PreparedSpawns[*CandidateIndex]))
						{
							bCanExtendRow = false;
							break;
						}
					}

					if (!bCanExtendRow)
					{
						break;
					}

					++RectHeight;
				}

				for (int32 OffsetY = 0; OffsetY < RectHeight; ++OffsetY)
				{
					for (int32 OffsetX = 0; OffsetX < RectWidth; ++OffsetX)
					{
						if ((OffsetX == 0) && (OffsetY == 0))
						{
							continue;
						}

						const int32* CandidateIndex = SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX + OffsetX, CurrentRun.TileY + OffsetY));
						if (CandidateIndex != nullptr)
						{
							bConsumed[*CandidateIndex] = true;
						}
					}
				}

				CurrentRun.TileSpanX = RectWidth;
				CurrentRun.TileSpanY = RectHeight;

				if ((RectWidth > 1) || (RectHeight > 1))
				{
					const FPreparedSpawn& EndSpawn = PreparedSpawns[*SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX + RectWidth - 1, CurrentRun.TileY + RectHeight - 1))];
					CurrentRun.LocalSpawnTransform.SetLocation(BuildBottomAnchoredMergedLocation(CurrentRun, EndSpawn));
				}
			}
			else if (IsVerticalMergeCandidate(CurrentRun))
			{
				int32 RunHeight = 1;
				while (true)
				{
					const int32 NextTileY = CurrentRun.TileY + RunHeight;
					const int32* NextIndex = SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX, NextTileY));
					if ((NextIndex == nullptr) || bConsumed[*NextIndex] || !ArePreparedSpawnsMergeCompatible(CurrentRun, PreparedSpawns[*NextIndex]))
					{
						break;
					}

					bConsumed[*NextIndex] = true;
					++RunHeight;
				}

				CurrentRun.TileSpanY = RunHeight;
				if (RunHeight > 1)
				{
					const FPreparedSpawn& EndSpawn = PreparedSpawns[*SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX, CurrentRun.TileY + RunHeight - 1))];
					CurrentRun.LocalSpawnTransform.SetLocation(BuildBottomAnchoredMergedLocation(CurrentRun, EndSpawn));
				}
			}
			else if (IsHorizontalMergeCandidate(CurrentRun))
			{
				int32 RunWidth = 1;
				while (true)
				{
					const int32 NextTileX = CurrentRun.TileX + RunWidth;
					const int32* NextIndex = SpawnIndexByCell.Find(FIntPoint(NextTileX, CurrentRun.TileY));
					if ((NextIndex == nullptr) || bConsumed[*NextIndex] || !CanMergeHorizontally(CurrentRun, PreparedSpawns[*NextIndex]))
					{
						break;
					}

					bConsumed[*NextIndex] = true;
					++RunWidth;
				}

				CurrentRun.TileSpanX = RunWidth;
				if (RunWidth > 1)
				{
					const FPreparedSpawn& EndSpawn = PreparedSpawns[*SpawnIndexByCell.Find(FIntPoint(CurrentRun.TileX + RunWidth - 1, CurrentRun.TileY))];
					CurrentRun.LocalSpawnTransform.SetLocation(BuildMergedCenterLocation(CurrentRun, EndSpawn));
				}
			}

			MergedPreparedSpawns.Add(CurrentRun);
		}

		return MergedPreparedSpawns;
	}

	FTransform BuildSpawnTransform(
		const UPaperTileMap& TileMap,
		int32 TileX,
		int32 TileY,
		const FResolvedTileCell& Cell,
		float SourceTileHeightInUU,
		const FVector2D& LocationScaleXZ)
	{
		const FVector TileCenter = TileMap.GetTileCenterInLocalSpace(TileX, TileY, 0);
		const FTransform TilePermutationTransform = BuildTilePermutationTransform(Cell.TileInfo);
		const FVector RootOffset(0.0f, 0.0f, -SourceTileHeightInUU * 0.5f);
		FVector RootLocation = TileCenter + TilePermutationTransform.TransformPosition(RootOffset);
		RootLocation.X *= LocationScaleXZ.X;
		RootLocation.Z *= LocationScaleXZ.Y;

		return FTransform(
			TilePermutationTransform.GetRotation(),
			FVector(RootLocation.X, 0.0f, RootLocation.Z),
			TilePermutationTransform.GetScale3D());
	}

	void ConfigureSpawnedActor(
		AActor& SpawnedActor,
		const FPreparedSpawn& PreparedSpawn,
		const FVector& RequestedBlockSize)
	{
		const FVector ResolvedSolidActorSize = BuildSolidActorSize(RequestedBlockSize, PreparedSpawn.TileSpanX);
		const FVector ResolvedRectActorSize = BuildRectActorSize(RequestedBlockSize, PreparedSpawn.TileSpanX, PreparedSpawn.TileSpanY);
		const FVector ResolvedDeepRectActorSize = BuildDeepRectActorSize(RequestedBlockSize, PreparedSpawn.TileSpanX, PreparedSpawn.TileSpanY);

		switch (PreparedSpawn.SpawnRecipe.ActorKind)
		{
		case EImportedActorKind::Ramp:
			if (APlatformerRamp* Ramp = Cast<APlatformerRamp>(&SpawnedActor))
			{
				Ramp->SetRampSize(ResolvedSolidActorSize);
				if (PreparedSpawn.RampAngleDegrees > 0)
				{
					Ramp->SetRampAngleDegrees(PreparedSpawn.RampAngleDegrees);
				}
			}
			break;

		case EImportedActorKind::Block:
			if (APlatformerBlock* Block = Cast<APlatformerBlock>(&SpawnedActor))
			{
				Block->SetBlockSize(ResolvedSolidActorSize);
				Block->SetBlockMeshVariant(PreparedSpawn.BlockMeshVariant);
			}
			break;

		case EImportedActorKind::SoftPlatform:
			if (APlatformerSoftPlatform* SoftPlatform = Cast<APlatformerSoftPlatform>(&SpawnedActor))
			{
				SoftPlatform->SetPlatformSize(ResolvedSolidActorSize);
			}
			break;

		case EImportedActorKind::Spikes:
			if (APlatformerSpikes* Spikes = Cast<APlatformerSpikes>(&SpawnedActor))
			{
				Spikes->SetSpikeSize(ResolvedSolidActorSize);
			}
			break;

		case EImportedActorKind::TriggeredLift:
			if (APlatformerTriggeredLift* TriggeredLift = Cast<APlatformerTriggeredLift>(&SpawnedActor))
			{
				TriggeredLift->SetPlatformSize(ResolvedSolidActorSize);
				TriggeredLift->SetTriggerSize(FVector(ResolvedSolidActorSize.X, ResolvedSolidActorSize.Y, RequestedBlockSize.Z));
			}
			break;

		case EImportedActorKind::MovingPlatform:
			if (APlatformerMovingPlatform* MovingPlatform = Cast<APlatformerMovingPlatform>(&SpawnedActor))
			{
				MovingPlatform->SetPlatformSize(ResolvedSolidActorSize);
			}
			break;

		case EImportedActorKind::Stream:
			if (APlatformerStream* Stream = Cast<APlatformerStream>(&SpawnedActor))
			{
				Stream->SetVolumeSize(ResolvedDeepRectActorSize);
			}
			break;

		case EImportedActorKind::GravityVolume:
			if (APlatformerGravityVolume* GravityVolume = Cast<APlatformerGravityVolume>(&SpawnedActor))
			{
				GravityVolume->SetVolumeSize(ResolvedDeepRectActorSize);
			}
			break;

		case EImportedActorKind::Ladder:
			if (APlatformerLadder* Ladder = Cast<APlatformerLadder>(&SpawnedActor))
			{
				Ladder->SetLadderSize(ResolvedRectActorSize);
			}
			break;

		case EImportedActorKind::Switch:
			if (APlatformerSwitch* Switch = Cast<APlatformerSwitch>(&SpawnedActor))
			{
				Switch->SetTriggerExtent(RequestedBlockSize * 0.5f);
			}
			break;

		case EImportedActorKind::PlayerStart:
		case EImportedActorKind::GenericActor:
		default:
			break;
		}
	}

	void DestroyPreviouslyGeneratedActors(UWorld& World)
	{
		TArray<AActor*> ActorsToDestroy;

		for (ULevel* Level : World.GetLevels())
		{
			if (Level == nullptr)
			{
				continue;
			}

			for (AActor* Actor : Level->Actors)
			{
				if ((Actor != nullptr) && Actor->Tags.Contains(GeneratedActorTag))
				{
					ActorsToDestroy.Add(Actor);
				}
			}
		}

		for (AActor* Actor : ActorsToDestroy)
		{
			World.EditorDestroyActor(Actor, true);
		}
	}

	float GetSupportSurfaceLocalZ(const FPreparedSpawn& PreparedSpawn, const FVector& SolidActorSize)
	{
		switch (PreparedSpawn.SpawnRecipe.ActorKind)
		{
		case EImportedActorKind::Block:
		case EImportedActorKind::SoftPlatform:
		case EImportedActorKind::Ramp:
		case EImportedActorKind::MovingPlatform:
		case EImportedActorKind::TriggeredLift:
			return PreparedSpawn.LocalSpawnTransform.GetLocation().Z + SolidActorSize.Z;

		case EImportedActorKind::Spikes:
		case EImportedActorKind::Ladder:
		case EImportedActorKind::PlayerStart:
		case EImportedActorKind::Stream:
		case EImportedActorKind::GravityVolume:
		case EImportedActorKind::Switch:
		case EImportedActorKind::GenericActor:
		default:
			return PreparedSpawn.LocalSpawnTransform.GetLocation().Z;
		}
	}

	bool TryFindPlayerStartAnchorLocation(
		const TArray<FPreparedSpawn>& PreparedSpawns,
		const FVector& SolidActorSize,
		FVector& OutAnchorLocation,
		float& OutSupportSurfaceLocalZ,
		int32& OutPlayerStartIndex)
	{
		TArray<int32> PlayerStartIndices;

		for (int32 SpawnIndex = 0; SpawnIndex < PreparedSpawns.Num(); ++SpawnIndex)
		{
			if (PreparedSpawns[SpawnIndex].SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart)
			{
				PlayerStartIndices.Add(SpawnIndex);
			}
		}

		if (PlayerStartIndices.IsEmpty())
		{
			return false;
		}

		OutPlayerStartIndex = PlayerStartIndices[0];
		const FPreparedSpawn& PlayerStartSpawn = PreparedSpawns[OutPlayerStartIndex];
		const float PlayerStartLocalZ = PlayerStartSpawn.LocalSpawnTransform.GetLocation().Z;
		const int32 PlayerStartColumn = PlayerStartSpawn.TileX;

		bool bFoundSupportActor = false;
		float HighestSupportSurfaceZ = -FLT_MAX;
		FVector SupportLocation = PlayerStartSpawn.LocalSpawnTransform.GetLocation();
		float SupportSurfaceLocalZ = PlayerStartLocalZ;

		for (int32 SpawnIndex = 0; SpawnIndex < PreparedSpawns.Num(); ++SpawnIndex)
		{
			if (SpawnIndex == OutPlayerStartIndex)
			{
				continue;
			}

			const FPreparedSpawn& CandidateSpawn = PreparedSpawns[SpawnIndex];
			if (CandidateSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart)
			{
				continue;
			}

			if (CandidateSpawn.TileX != PlayerStartColumn)
			{
				continue;
			}

			const float CandidateSupportSurfaceZ = GetSupportSurfaceLocalZ(CandidateSpawn, SolidActorSize);
			if (CandidateSupportSurfaceZ > (PlayerStartLocalZ + KINDA_SMALL_NUMBER))
			{
				continue;
			}

			if (!bFoundSupportActor || (CandidateSupportSurfaceZ > HighestSupportSurfaceZ))
			{
				bFoundSupportActor = true;
				HighestSupportSurfaceZ = CandidateSupportSurfaceZ;
				SupportLocation = CandidateSpawn.LocalSpawnTransform.GetLocation();
				SupportSurfaceLocalZ = CandidateSupportSurfaceZ;
			}
		}

		OutAnchorLocation = SupportLocation;
		OutSupportSurfaceLocalZ = SupportSurfaceLocalZ;
		return true;
	}

	bool BuildLevelFromTileMapAsset(
		UPaperTileMap& TileMap,
		const UTileSetAsset* ImportMappingAsset,
		const FString& SourceTileMapObjectPath,
		const FString& OutputLevelPackagePath,
		const FPaperTileMapGenerationSettings& GenerationSettings,
		FString& OutError)
	{
		if (TileMap.ProjectionMode != ETileMapProjectionMode::Orthogonal)
		{
			OutError = FString::Printf(
				TEXT("PaperTileMap %s uses a non-orthogonal projection. The importer currently supports orthogonal maps only."),
				*SourceTileMapObjectPath);
			return false;
		}

		if (ImportMappingAsset == nullptr)
		{
			UE_LOG(
				LogCookieBrosPaperTileMapImport,
				Warning,
				TEXT("No Paper Tile import mapping asset was provided. Unmapped tiles will fall back to APlatformerBlock."));
		}

		UWorld* World = nullptr;
		FString OutputLevelFilename;
		if (!FPackageName::TryConvertLongPackageNameToFilename(
				OutputLevelPackagePath,
				OutputLevelFilename,
				FPackageName::GetMapPackageExtension()))
		{
			OutError = FString::Printf(TEXT("Failed to convert level path to filename: %s"), *OutputLevelPackagePath);
			return false;
		}

		if (FPaths::FileExists(OutputLevelFilename))
		{
			World = UEditorLoadingAndSavingUtils::LoadMap(OutputLevelFilename);
		}
		else
		{
			World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
		}

		if (World == nullptr)
		{
			OutError = FString::Printf(TEXT("Failed to prepare output level: %s"), *OutputLevelPackagePath);
			return false;
		}

		DestroyPreviouslyGeneratedActors(*World);

		if (AWorldSettings* WorldSettings = World->GetWorldSettings())
		{
			if (GenerationSettings.GameModeOverrideClass)
			{
				WorldSettings->Modify();
				WorldSettings->DefaultGameMode = GenerationSettings.GameModeOverrideClass;
			}
		}

		const FVector RequestedBlockSize = SanitizeBlockSize(GenerationSettings.BlockSize);
		const float SourceTileWidthInUU = FMath::Max(TileMap.TileWidth * TileMap.GetUnrealUnitsPerPixel(), 1.0f);
		const float SourceTileHeightInUU = FMath::Max(TileMap.TileHeight * TileMap.GetUnrealUnitsPerPixel(), 1.0f);
		const FVector SolidActorSize = BuildSolidActorSize(RequestedBlockSize);
		const FVector2D LocationScaleXZ(
			RequestedBlockSize.X / SourceTileWidthInUU,
			RequestedBlockSize.Z / SourceTileHeightInUU);

		TArray<FPreparedSpawn> PreparedSpawns;
		PreparedSpawns.Reserve(TileMap.MapWidth * TileMap.MapHeight);

		TSet<FName> WarnedUserDataNames;
		for (int32 TileY = 0; TileY < TileMap.MapHeight; ++TileY)
		{
			for (int32 TileX = 0; TileX < TileMap.MapWidth; ++TileX)
			{
				FResolvedTileCell ResolvedCell;
				if (!TryGetResolvedTileCell(TileMap, TileX, TileY, ResolvedCell))
				{
					continue;
				}

				if (ResolvedCell.RampDescriptor.bIsRamp && (ResolvedCell.RampDescriptor.SegmentIndex > 0))
				{
					continue;
				}

				FSpawnRecipe SpawnRecipe;
				if (!TryResolveImportMappingSpawnRecipe(ImportMappingAsset, ResolvedCell.UserDataName, SpawnRecipe))
				{
					if (!ResolvedCell.UserDataName.IsNone() && !WarnedUserDataNames.Contains(ResolvedCell.UserDataName))
					{
						WarnedUserDataNames.Add(ResolvedCell.UserDataName);
						UE_LOG(
							LogCookieBrosPaperTileMapImport,
							Warning,
							TEXT("Unsupported TileSet UserDataName '%s'. Falling back to APlatformerBlock."),
							*ResolvedCell.UserDataName.ToString());
					}

					SpawnRecipe.ActorClass = APlatformerBlock::StaticClass();
					SpawnRecipe.ActorKind = EImportedActorKind::Block;
					SpawnRecipe.DebugName = FName(TEXT("BlockFallback"));
				}

				FPreparedSpawn& PreparedSpawn = PreparedSpawns.AddDefaulted_GetRef();
				PreparedSpawn.TileX = TileX;
				PreparedSpawn.TileY = TileY;
				PreparedSpawn.BlockMeshVariant = ResolvedCell.BlockDescriptor.MeshVariant;
				PreparedSpawn.RampAngleDegrees = ResolvedCell.RampDescriptor.bIsRamp ? ResolvedCell.RampDescriptor.AngleDegrees : INDEX_NONE;
				PreparedSpawn.ResolvedCell = ResolvedCell;
				PreparedSpawn.SpawnRecipe = SpawnRecipe;
				PreparedSpawn.LocalSpawnTransform = BuildSpawnTransform(
					TileMap,
					TileX,
					TileY,
					ResolvedCell,
					SourceTileHeightInUU,
					LocationScaleXZ);
			}
		}

		FVector LevelTranslation = FVector::ZeroVector;
		int32 PrimaryPlayerStartIndex = INDEX_NONE;
		float PrimaryPlayerStartSupportSurfaceWorldZ = 0.0f;
		FVector PlayerStartAnchorLocation = FVector::ZeroVector;
		float PlayerStartSupportSurfaceLocalZ = 0.0f;
		if (TryFindPlayerStartAnchorLocation(
			PreparedSpawns,
			SolidActorSize,
			PlayerStartAnchorLocation,
			PlayerStartSupportSurfaceLocalZ,
			PrimaryPlayerStartIndex))
		{
			LevelTranslation = -PlayerStartAnchorLocation;
			PrimaryPlayerStartSupportSurfaceWorldZ = PlayerStartSupportSurfaceLocalZ + LevelTranslation.Z;

			int32 PlayerStartCount = 0;
			for (const FPreparedSpawn& PreparedSpawn : PreparedSpawns)
			{
				PlayerStartCount += (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart) ? 1 : 0;
			}

			if (PlayerStartCount > 1)
			{
				UE_LOG(
					LogCookieBrosPaperTileMapImport,
					Warning,
					TEXT("Multiple PlayerStart tiles were found. Only the first one will be used."));
			}
		}
		(void)PrimaryPlayerStartIndex;

		const TArray<FPreparedSpawn> FinalPreparedSpawns = BuildMergedPreparedSpawns(PreparedSpawns);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APlatformerLightningBase* LightingActor = World->SpawnActor<APlatformerLightningBase>(
			APlatformerLightningBase::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);

		if (LightingActor == nullptr)
		{
			OutError = TEXT("Failed to spawn PlatformerLightningBase.");
			return false;
		}

		LightingActor->Tags.AddUnique(GeneratedActorTag);
		LightingActor->SetFolderPath(TEXT("Generated/PaperTileMapSetup"));
		LightingActor->SetActorLabel(TEXT("PlatformerLightningBase"));

		int32 SpawnedActorCount = 1;
		bool bSpawnedPrimaryPlayerStart = false;
		bool bEncounteredPrimaryPlayerStart = false;

		for (int32 SpawnIndex = 0; SpawnIndex < FinalPreparedSpawns.Num(); ++SpawnIndex)
		{
			const FPreparedSpawn& PreparedSpawn = FinalPreparedSpawns[SpawnIndex];
			const bool bIsPrimaryPlayerStart = (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart)
				&& !bEncounteredPrimaryPlayerStart;

			if ((PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart) && !bIsPrimaryPlayerStart)
			{
				continue;
			}

			FVector FinalLocation = PreparedSpawn.LocalSpawnTransform.GetLocation() + LevelTranslation;
			if (bIsPrimaryPlayerStart)
			{
				FinalLocation = FVector::ZeroVector;
			}
			else if (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::Ladder)
			{
				FinalLocation.Y -= RequestedBlockSize.Y;
			}

			AActor* SpawnedActor = World->SpawnActor<AActor>(
				PreparedSpawn.SpawnRecipe.ActorClass,
				FinalLocation,
				PreparedSpawn.LocalSpawnTransform.Rotator(),
				SpawnParameters);

			if (SpawnedActor == nullptr)
			{
				OutError = FString::Printf(TEXT("Failed to spawn environment actor at cell (%d, %d)."), PreparedSpawn.TileX, PreparedSpawn.TileY);
				return false;
			}

			if (PreparedSpawn.SpawnRecipe.ActorKind != EImportedActorKind::PlayerStart)
			{
				SpawnedActor->SetActorScale3D(PreparedSpawn.LocalSpawnTransform.GetScale3D());
			}

			ConfigureSpawnedActor(*SpawnedActor, PreparedSpawn, RequestedBlockSize);
			SpawnedActor->Tags.AddUnique(GeneratedActorTag);
			SpawnedActor->SetFolderPath(
				PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart
					? TEXT("Generated/PaperTileMapSetup")
					: TEXT("Generated/PaperTileMapBlocks"));
			const bool bIsMergedHorizontally = PreparedSpawn.TileSpanX > 1;
			const bool bIsMergedVertically = PreparedSpawn.TileSpanY > 1;
			const FString ActorLabel = (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart)
				? FString(TEXT("PlayerStart"))
				: ((bIsMergedHorizontally || bIsMergedVertically)
					? FString::Printf(
						TEXT("PTM_%s_%02d_%02d_x%d_y%d"),
						*PreparedSpawn.SpawnRecipe.DebugName.ToString(),
						PreparedSpawn.TileX,
						PreparedSpawn.TileY,
						PreparedSpawn.TileSpanX,
						PreparedSpawn.TileSpanY)
					: FString::Printf(
						TEXT("PTM_%s_%02d_%02d"),
						*PreparedSpawn.SpawnRecipe.DebugName.ToString(),
						PreparedSpawn.TileX,
						PreparedSpawn.TileY));
			SpawnedActor->SetActorLabel(ActorLabel);
			SpawnedActor->RerunConstructionScripts();
			++SpawnedActorCount;

			if (bIsPrimaryPlayerStart)
			{
				const FBox PlayerStartBounds = SpawnedActor->GetComponentsBoundingBox(true);
				if (PlayerStartBounds.IsValid)
				{
					FVector AdjustedLocation = SpawnedActor->GetActorLocation();
					AdjustedLocation.Z += PrimaryPlayerStartSupportSurfaceWorldZ - PlayerStartBounds.Min.Z;
					SpawnedActor->SetActorLocation(AdjustedLocation, false, nullptr, ETeleportType::TeleportPhysics);
				}

				bSpawnedPrimaryPlayerStart = true;
			}

			if (PreparedSpawn.SpawnRecipe.ActorKind == EImportedActorKind::PlayerStart)
			{
				bEncounteredPrimaryPlayerStart = true;
			}
		}

		if (!UEditorLoadingAndSavingUtils::SaveMap(World, OutputLevelPackagePath))
		{
			OutError = FString::Printf(TEXT("Failed to save generated level: %s"), *OutputLevelPackagePath);
			return false;
		}

		UE_LOG(
			LogCookieBrosPaperTileMapImport,
			Display,
			TEXT("[PaperTileMap->Level] Source=%s, Output=%s, SpawnedActors=%d, SourceTileSizeUU=(%.3f, %.3f), BlockSize=(%.3f, %.3f, %.3f), BlockDepthY=%.3f, PlayerStart=%s, GameModeOverride=%s"),
			*SourceTileMapObjectPath,
			*OutputLevelPackagePath,
			SpawnedActorCount,
			SourceTileWidthInUU,
			SourceTileHeightInUU,
			RequestedBlockSize.X,
			RequestedBlockSize.Y,
			RequestedBlockSize.Z,
			ImportedBlockDepth,
			bSpawnedPrimaryPlayerStart ? TEXT("Yes") : TEXT("No"),
			GenerationSettings.GameModeOverrideClass ? *GenerationSettings.GameModeOverrideClass->GetPathName() : TEXT("None"));

		return true;
	}

	bool BuildLevelFromTileMap(const FImportArgs& Args, FString& OutError)
	{
		UPaperTileMap* TileMap = LoadTileMapAsset(Args.SourceTileMapObjectPath);
		if (TileMap == nullptr)
		{
			OutError = FString::Printf(TEXT("Failed to load PaperTileMap asset: %s"), *Args.SourceTileMapObjectPath);
			return false;
		}

		UTileSetAsset* ImportMappingAsset = nullptr;
		if (!Args.ImportMappingObjectPath.IsEmpty())
		{
			ImportMappingAsset = LoadImportMappingAsset(Args.ImportMappingObjectPath);
			if (ImportMappingAsset == nullptr)
			{
				OutError = FString::Printf(TEXT("Failed to load import mapping asset: %s"), *Args.ImportMappingObjectPath);
				return false;
			}
		}

		const TSubclassOf<AGameModeBase> GameModeOverrideClass = LoadGameModeClass(Args.GameModeClassObjectPath);
		if (!Args.GameModeClassObjectPath.IsEmpty() && !GameModeOverrideClass)
		{
			OutError = FString::Printf(TEXT("Failed to load GameMode class: %s"), *Args.GameModeClassObjectPath);
			return false;
		}

		FPaperTileMapGenerationSettings GenerationSettings = Args.GenerationSettings;
		GenerationSettings.GameModeOverrideClass = GameModeOverrideClass;

		return BuildLevelFromTileMapAsset(
			*TileMap,
			ImportMappingAsset,
			Args.SourceTileMapObjectPath,
			Args.OutputLevelPackagePath,
			GenerationSettings,
			OutError);
	}
}

bool FCookieBrosPaperTileMapLevelImporter::HasCommandLineRequest()
{
	FString IgnoredValue;
	return FParse::Value(FCommandLine::Get(), TEXT("CookieBrosImportPaperTileMap="), IgnoredValue);
}

bool FCookieBrosPaperTileMapLevelImporter::RunFromCommandLine()
{
	using namespace CookieBrosPaperTileMapImport;

	FImportArgs Args;
	FString ErrorMessage;
	const bool bParsedArguments = TryParseCommandLine(Args, ErrorMessage);
	const bool bExitAfterImport = FParse::Param(FCommandLine::Get(), TEXT("CookieBrosExitAfterImport"));

	bool bSuccess = false;
	if (!bParsedArguments)
	{
		UE_LOG(LogCookieBrosPaperTileMapImport, Error, TEXT("%s"), *ErrorMessage);
	}
	else
	{
		bSuccess = BuildLevelFromTileMap(Args, ErrorMessage);
		if (!bSuccess)
		{
			UE_LOG(LogCookieBrosPaperTileMapImport, Error, TEXT("%s"), *ErrorMessage);
		}
	}

	if (bExitAfterImport)
	{
		FPlatformMisc::RequestExitWithStatus(false, bSuccess ? 0 : 1);
	}

	return bSuccess;
}

bool FCookieBrosPaperTileMapLevelImporter::GenerateLevelFromTileMapAsset(
	UPaperTileMap* TileMap,
	const UTileSetAsset* ImportMappingAsset,
	const FString& OutputLevelPackagePath,
	const FPaperTileMapGenerationSettings& GenerationSettings,
	FString& OutError)
{
	using namespace CookieBrosPaperTileMapImport;

	if (TileMap == nullptr)
	{
		OutError = TEXT("GenerateLevelFromTileMapAsset received a null TileMap.");
		return false;
	}

	const FString NormalizedOutputLevelPackagePath = NormalizePackagePath(OutputLevelPackagePath);
	if (!FPackageName::IsValidLongPackageName(NormalizedOutputLevelPackagePath))
	{
		OutError = FString::Printf(TEXT("Invalid output level path: %s"), *OutputLevelPackagePath);
		return false;
	}

	return BuildLevelFromTileMapAsset(
		*TileMap,
		ImportMappingAsset,
		GetTileMapObjectPath(*TileMap),
		NormalizedOutputLevelPackagePath,
		GenerationSettings,
		OutError);
}
