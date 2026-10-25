#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EinsteinHatTypes.h"
#include "EinsteinHatLibrary.generated.h"

/** Blueprint-facing access to the header-only geometry in EinsteinHatTypes.h. */
UCLASS()
class EINSTEINHATTILER_API UEinsteinHatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The thirteen outline vertices of the hat in centimetres, on the actor's XY plane. */
	UFUNCTION(BlueprintPure, Category = "Einstein Hat")
	static TArray<FVector2D> GetHatOutline(float TileScale = 100.f);

	/** Centre of each of the eight kites, in centimetres. */
	UFUNCTION(BlueprintPure, Category = "Einstein Hat")
	static TArray<FVector2D> GetHatKiteCentres(float TileScale = 100.f);

	/** How many kites make up a hat. */
	UFUNCTION(BlueprintPure, Category = "Einstein Hat")
	static int32 GetHatKiteCount();

	/** Triangulates a full hat into local space. */
	UFUNCTION(BlueprintCallable, Category = "Einstein Hat")
	static void BuildHatMesh(const FEinsteinHatConfig& Config, TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs);

	/** Triangulates the single unit kite into local space. */
	UFUNCTION(BlueprintCallable, Category = "Einstein Hat")
	static void BuildKiteMesh(const FEinsteinHatConfig& Config, TArray<FVector>& OutVertices,
		TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs);
};
