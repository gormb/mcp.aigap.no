#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EinsteinHatTypes.h"
#include "KiteActor.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

/**
 * The smallest physical unit of the tiler: one right kite built as a triangle fan around
 * its long-diagonal midpoint, so KiteMidpointHeight raises or lowers that midpoint.
 */
UCLASS(Blueprintable)
class EINSTEINHATTILER_API AKiteActor : public AActor
{
	GENERATED_BODY()

public:
	AKiteActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Einstein Hat")
	TObjectPtr<UProceduralMeshComponent> KiteMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	FEinsteinHatConfig Config;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Einstein Hat")
	TObjectPtr<UMaterialInterface> TileMaterial;

	/** Rebuilds the kite mesh from Config. Call after changing Config at runtime. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Einstein Hat")
	void RebuildMesh();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
