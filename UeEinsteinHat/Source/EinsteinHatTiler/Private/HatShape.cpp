#include "HatShape.h"

#include "EinsteinHatLibrary.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"

AHatShape::AHatShape()
{
	PrimaryActorTick.bCanEverTick = false;

	HatMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("HatMesh"));
	HatMesh->SetupAttachment(RootComponent);
	HatMesh->bUseAsyncCooking = true;
	SetRootComponent(HatMesh);
}

void AHatShape::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildMesh();
}

FVector AHatShape::GetLocalCentre() const
{
	const TArray<FVector2D> Outline = UEinsteinHatLibrary::GetHatOutline(Config.TileScale);
	if (Outline.Num() == 0)
	{
		return FVector::ZeroVector;
	}

	FVector2D Min = Outline[0];
	FVector2D Max = Outline[0];
	for (const FVector2D& Point : Outline)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
		Max.X = FMath::Max(Max.X, Point.X);
		Max.Y = FMath::Max(Max.Y, Point.Y);
	}

	return FVector((Min.X + Max.X) * 0.5, (Min.Y + Max.Y) * 0.5, 0.f);
}

void AHatShape::RebuildMesh()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	UEinsteinHatLibrary::BuildHatMesh(Config, Vertices, Triangles, Normals, UVs);

	HatMesh->ClearAllMeshSections();
	if (Vertices.Num() == 0 || Triangles.Num() == 0)
	{
		return;
	}

	// Centre on the actor origin, then apply the deterministic seeded yaw.
	const FVector Centre = GetLocalCentre();
	const FRotator SeedRotation(0.f, Seed != 0 ? FRandomStream(Seed).FRandRange(0.f, 360.f) : 0.f, 0.f);
	for (FVector& Vertex : Vertices)
	{
		Vertex -= Centre;
		Vertex = SeedRotation.RotateVector(Vertex);
	}

	HatMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs,
		TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);

	if (TileMaterial)
	{
		HatMesh->SetMaterial(0, TileMaterial);
	}
}
