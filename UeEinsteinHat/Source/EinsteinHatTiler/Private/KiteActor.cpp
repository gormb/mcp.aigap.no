#include "KiteActor.h"

#include "EinsteinHatLibrary.h"
#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"

AKiteActor::AKiteActor()
{
	PrimaryActorTick.bCanEverTick = false;

	KiteMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("KiteMesh"));
	KiteMesh->SetupAttachment(RootComponent);
	KiteMesh->bUseAsyncCooking = true;
	SetRootComponent(KiteMesh);
}

void AKiteActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildMesh();
}

void AKiteActor::RebuildMesh()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	UEinsteinHatLibrary::BuildKiteMesh(Config, Vertices, Triangles, Normals, UVs);

	// Centre the kite on the actor origin so instances pivot around their own midpoint.
	const FVector2D Centre = EinsteinHatGeometry::KiteCentrePlane(EinsteinHatGeometry::GetUnitKite())
		* Config.TileScale;
	for (FVector& Vertex : Vertices)
	{
		Vertex.X -= Centre.X;
		Vertex.Y -= Centre.Y;
	}

	KiteMesh->ClearAllMeshSections();
	if (Vertices.Num() == 0 || Triangles.Num() == 0)
	{
		return;
	}

	KiteMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs,
		TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);

	if (TileMaterial)
	{
		KiteMesh->SetMaterial(0, TileMaterial);
	}
}
