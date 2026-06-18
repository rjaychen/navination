#include "MazeGameInstance.h"

void UMazeGameInstance::UpdateBestLevel(int32 Level)
{
    BestLevel = FMath::Max(BestLevel, Level);
}
