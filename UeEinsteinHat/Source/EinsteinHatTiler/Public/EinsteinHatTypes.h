#pragma once

#include "CoreMinimal.h"
#include "EinsteinHatTypes.generated.h"

/**
 * Which monotile family the tiler builds.
 * Only Hat is implemented; Turtle and Spectre are reserved for DEV.md milestone IX.
 */
UENUM(BlueprintType)
enum class EEinsteinHatVariant : uint8
{
	Hat UMETA(DisplayName = "Hat"),
	Turtle UMETA(DisplayName = "Turtle (reserved)"),
	Spectre UMETA(DisplayName = "Spectre (reserved)")
};

/**
 * Which implementation produces the geometry. AssemblyCoded is a code-path selector, not a
 * quality setting: both paths must produce identical output. See DEV.md section 4.5.
 */
UENUM(BlueprintType)
enum class EEinsteinHatCodePath : uint8
{
	/** Portable scalar C++. The reference implementation and the fallback. */
	Cpp UMETA(DisplayName = "C++"),
	/** Hand-vectorised intrinsics, 256/512-bit SIMD selected at runtime on CPU support. */
	AssemblyCoded UMETA(DisplayName = "Assembly Coded")
};

/** A vertex of the triangular lattice the hat polykite is built on. */
USTRUCT(BlueprintType)
struct FEinsteinHatLatticePoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	int32 Y = 0;

	FEinsteinHatLatticePoint() = default;
	FEinsteinHatLatticePoint(int32 InX, int32 InY) : X(InX), Y(InY) {}

	bool operator==(const FEinsteinHatLatticePoint& Other) const { return X == Other.X && Y == Other.Y; }
};

/**
 * One right kite of the polykite: 60 deg apex, 90 deg, 120 deg apex, 90 deg, counter-clockwise.
 * The hat is exactly eight of these.
 */
USTRUCT(BlueprintType)
struct FEinsteinHatKite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	TArray<FEinsteinHatLatticePoint> Vertices;

	FEinsteinHatKite() = default;
	explicit FEinsteinHatKite(const TArray<FEinsteinHatLatticePoint>& InVertices) : Vertices(InVertices) {}
};

/** Elevation and scale knobs shared by AKiteActor and AHatShape (DEV.md section 4.3). */
USTRUCT(BlueprintType)
struct FEinsteinHatConfig
{
	GENERATED_BODY()

	/** Size of one lattice unit in centimetres. A whole hat is roughly 5 lattice units across. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat", meta = (ClampMin = "0.001"))
	float TileScale = 100.f;

	/** Raises (+) or lowers (-) the centre point of every kite. Milestone VII. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	float KiteMidpointHeight = 0.f;

	/** Raises (+) or lowers (-) every vertex of the kite outlines. Milestone VIII. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	float PointHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	EEinsteinHatVariant Variant = EEinsteinHatVariant::Hat;

	/** Geometry backend. Orthogonal to the shape settings above — see DEV.md section 4.5. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat|Performance")
	EEinsteinHatCodePath CodePath = EEinsteinHatCodePath::Cpp;
};

/**
 * Header-only geometry "DNA" of the hat tile.
 *
 * Lattice coordinates are integers on the triangular lattice; LatticeToPlane(x, y) maps them to
 * the plane as (x + y/2, y * sqrt(3)/2), the convention used by the reference implementations
 * listed in DEV.md section 5.
 *
 * The eight kites below were verified to tile the thirteen-vertex outline exactly: the symmetric
 * difference between their union and the outline is zero.
 */
namespace EinsteinHatGeometry
{
	/** sqrt(3) / 2, the height of a unit equilateral triangle. */
	inline constexpr double LatticeHeight = 0.86602540378443864676;

	inline FVector2D LatticeToPlane(const FEinsteinHatLatticePoint& Point)
	{
		return FVector2D(Point.X + 0.5 * Point.Y, LatticeHeight * Point.Y);
	}

	/** The thirteen outline vertices of the hat, counter-clockwise in lattice coordinates. */
	inline const TArray<FEinsteinHatLatticePoint>& GetHatOutline()
	{
		static const TArray<FEinsteinHatLatticePoint> Outline = {
			FEinsteinHatLatticePoint(0, 0),
			FEinsteinHatLatticePoint(-1, -1),
			FEinsteinHatLatticePoint(0, -2),
			FEinsteinHatLatticePoint(2, -2),
			FEinsteinHatLatticePoint(2, -1),
			FEinsteinHatLatticePoint(4, -2),
			FEinsteinHatLatticePoint(5, -1),
			FEinsteinHatLatticePoint(4, 0),
			FEinsteinHatLatticePoint(3, 0),
			FEinsteinHatLatticePoint(2, 2),
			FEinsteinHatLatticePoint(0, 3),
			FEinsteinHatLatticePoint(0, 2),
			FEinsteinHatLatticePoint(-1, 2)
		};
		return Outline;
	}

	/** The single kite every other shape is derived from. */
	inline const FEinsteinHatKite& GetUnitKite()
	{
		static const FEinsteinHatKite Kite(TArray<FEinsteinHatLatticePoint>{
			FEinsteinHatLatticePoint(0, 0),
			FEinsteinHatLatticePoint(2, -1),
			FEinsteinHatLatticePoint(2, 0),
			FEinsteinHatLatticePoint(1, 1)
		});
		return Kite;
	}

	/** The hat as eight congruent kites. */
	inline const TArray<FEinsteinHatKite>& GetHatKites()
	{
		static const TArray<FEinsteinHatKite> Kites = {
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(0, 0), FEinsteinHatLatticePoint(-1, -1), FEinsteinHatLatticePoint(0, -2), FEinsteinHatLatticePoint(1, -2) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(0, 0), FEinsteinHatLatticePoint(1, 1), FEinsteinHatLatticePoint(0, 2), FEinsteinHatLatticePoint(-1, 2) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(0, 0), FEinsteinHatLatticePoint(1, -2), FEinsteinHatLatticePoint(2, -2), FEinsteinHatLatticePoint(2, -1) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(0, 0), FEinsteinHatLatticePoint(2, -1), FEinsteinHatLatticePoint(2, 0), FEinsteinHatLatticePoint(1, 1) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(2, 2), FEinsteinHatLatticePoint(0, 3), FEinsteinHatLatticePoint(0, 2), FEinsteinHatLatticePoint(1, 1) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(2, 2), FEinsteinHatLatticePoint(1, 1), FEinsteinHatLatticePoint(2, 0), FEinsteinHatLatticePoint(3, 0) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(4, -2), FEinsteinHatLatticePoint(3, 0), FEinsteinHatLatticePoint(2, 0), FEinsteinHatLatticePoint(2, -1) }),
			FEinsteinHatKite(TArray<FEinsteinHatLatticePoint>{ FEinsteinHatLatticePoint(4, -2), FEinsteinHatLatticePoint(5, -1), FEinsteinHatLatticePoint(4, 0), FEinsteinHatLatticePoint(3, 0) })
		};
		return Kites;
	}

	/** Midpoint of a kite's long diagonal: from the 60 deg apex (0) to the 120 deg apex (2). */
	inline FVector2D KiteCentrePlane(const FEinsteinHatKite& Kite)
	{
		if (Kite.Vertices.Num() < 3)
		{
			return FVector2D::ZeroVector;
		}
		return (LatticeToPlane(Kite.Vertices[0]) + LatticeToPlane(Kite.Vertices[2])) * 0.5;
	}

	/**
	 * Appends one kite as a triangle fan around its long-diagonal midpoint, so that
	 * FEinsteinHatConfig::KiteMidpointHeight raises or lowers the centre of the kite.
	 */
	inline void AppendKite(const FEinsteinHatKite& Kite, const FEinsteinHatConfig& Config,
		TArray<FVector>& OutVertices, TArray<int32>& OutTriangles)
	{
		if (Kite.Vertices.Num() < 3)
		{
			return;
		}

		const int32 Base = OutVertices.Num();
		for (const FEinsteinHatLatticePoint& Point : Kite.Vertices)
		{
			const FVector2D Plane = LatticeToPlane(Point) * Config.TileScale;
			OutVertices.Add(FVector(Plane.X, Plane.Y, Config.PointHeight));
		}

		const FVector2D CentrePlane = KiteCentrePlane(Kite) * Config.TileScale;
		const int32 CentreIndex = OutVertices.Num();
		OutVertices.Add(FVector(CentrePlane.X, CentrePlane.Y, Config.PointHeight + Config.KiteMidpointHeight));

		const int32 Count = Kite.Vertices.Num();
		for (int32 Index = 0; Index < Count; ++Index)
		{
			OutTriangles.Add(CentreIndex);
			OutTriangles.Add(Base + Index);
			OutTriangles.Add(Base + ((Index + 1) % Count));
		}
	}
}
