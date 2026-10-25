#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EinsteinHatTypes.h"
#include "HatShape.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

/**
 * The container actor: builds a whole hat out of the eight kites defined in EinsteinHatTypes.h.
 *
 * Seed only rotates the shape (deterministic yaw); it is the hook for the seeded infinite-patch
 * generator described in DEV.md section 5.2 and milestone II.
 */
UCLASS(Blueprintable)
class EINSTEINHATTILER_API AHatShape : public AActor
{
	GENERATED_BODY()

public:
	AHatShape();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Einstein Hat")
	TObjectPtr<UProceduralMeshComponent> HatMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	FEinsteinHatConfig Config;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	TObjectPtr<UMaterialInterface> TileMaterial;

	/** Deterministic yaw applied to the tile. 0 disables the seeded rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	int32 Seed = 0;

	/** Rebuilds the hat mesh from Config and Seed. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Einstein Hat")
	void RebuildMesh();

	/** Centre of the hat in local space, in centimetres. */
	UFUNCTION(BlueprintPure, Category = "Einstein Hat")
	FVector GetLocalCentre() const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
