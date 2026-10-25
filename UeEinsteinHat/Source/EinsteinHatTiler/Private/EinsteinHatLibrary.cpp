#include "EinsteinHatLibrary.h"

namespace
{
	/** Planar UVs normalised by the mesh bounds so tile materials map predictably. */
	void BuildPlanarUVs(const TArray<FVector>& Vertices, TArray<FVector2D>& OutUVs)
	{
		OutUVs.Reset();
		if (Vertices.Num() == 0)
		{
			return;
		}

		FVector2D Min(Vertices[0].X, Vertices[0].Y);
		FVector2D Max = Min;
		for (const FVector& Vertex : Vertices)
		{
			Min.X = FMath::Min(Min.X, Vertex.X);
			Min.Y = FMath::Min(Min.Y, Vertex.Y);
			Max.X = FMath::Max(Max.X, Vertex.X);
			Max.Y = FMath::Max(Max.Y, Vertex.Y);
		}

		const FVector2D Size = Max - Min;
		const double SafeX = FMath::Max(Size.X, UE_DOUBLE_SMALL_NUMBER);
		const double SafeY = FMath::Max(Size.Y, UE_DOUBLE_SMALL_NUMBER);

		OutUVs.Reserve(Vertices.Num());
		for (const FVector& Vertex : Vertices)
		{
			OutUVs.Add(FVector2D((Vertex.X - Min.X) / SafeX, (Vertex.Y - Min.Y) / SafeY));
		}
	}

	void FillNormals(const TArray<FVector>& Vertices, TArray<FVector>& OutNormals)
	{
		OutNormals.Init(FVector::UpVector, Vertices.Num());
	}
}

TArray<FVector2D> UEinsteinHatLibrary::GetHatOutline(float TileScale)
{
	TArray<FVector2D> Result;
	const TArray<FEinsteinHatLatticePoint>& Outline = EinsteinHatGeometry::GetHatOutline();
	Result.Reserve(Outline.Num());
	for (const FEinsteinHatLatticePoint& Point : Outline)
	{
		Result.Add(EinsteinHatGeometry::LatticeToPlane(Point) * TileScale);
	}
	return Result;
}

TArray<FVector2D> UEinsteinHatLibrary::GetHatKiteCentres(float TileScale)
{
	TArray<FVector2D> Result;
	const TArray<FEinsteinHatKite>& Kites = EinsteinHatGeometry::GetHatKites();
	Result.Reserve(Kites.Num());
	for (const FEinsteinHatKite& Kite : Kites)
	{
		Result.Add(EinsteinHatGeometry::KiteCentrePlane(Kite) * TileScale);
	}
	return Result;
}

int32 UEinsteinHatLibrary::GetHatKiteCount()
{
	return EinsteinHatGeometry::GetHatKites().Num();
}

void UEinsteinHatLibrary::BuildHatMesh(const FEinsteinHatConfig& Config, TArray<FVector>& OutVertices,
	TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs)
{
	OutVertices.Reset();
	OutTriangles.Reset();

	if (Config.Variant != EEinsteinHatVariant::Hat)
	{
		// Turtle and Spectre are reserved (DEV.md milestone IX); fall back to the Hat so the
		// tiler always produces geometry.
		UE_LOG(LogTemp, Warning,
			TEXT("EinsteinHat: variant is not implemented yet, falling back to Hat."));
	}

	for (const FEinsteinHatKite& Kite : EinsteinHatGeometry::GetHatKites())
	{
		EinsteinHatGeometry::AppendKite(Kite, Config, OutVertices, OutTriangles);
	}

	FillNormals(OutVertices, OutNormals);
	BuildPlanarUVs(OutVertices, OutUVs);
}

void UEinsteinHatLibrary::BuildKiteMesh(const FEinsteinHatConfig& Config, TArray<FVector>& OutVertices,
	TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs)
{
	OutVertices.Reset();
	OutTriangles.Reset();

	EinsteinHatGeometry::AppendKite(EinsteinHatGeometry::GetUnitKite(), Config, OutVertices, OutTriangles);

	FillNormals(OutVertices, OutNormals);
	BuildPlanarUVs(OutVertices, OutUVs);
}
