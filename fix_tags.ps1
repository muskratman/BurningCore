$files = Get-ChildItem -Path 'c:\UE_projects\BurningCORE\Source\BurningCORE\GAS\Abilities\GA_*.cpp'
foreach ($f in $files) {
    $content = Get-Content $f.FullName -Raw
    $content = [regex]::Replace($content, 'AbilityTags\.AddTag\((FGameplayTag::RequestGameplayTag\(FName\(".*?"\)\))\);', 'SetAssetTags(FGameplayTagContainer($1));')
    $content = [regex]::Replace($content, 'ActivationOwnedTags\.AddTag\((FGameplayTag::RequestGameplayTag\(FName\(".*?"\)\))\);', 'SetActivationOwnedTags(FGameplayTagContainer($1));')
    $content = [regex]::Replace($content, 'ActivationBlockedTags\.AddTag\((FGameplayTag::RequestGameplayTag\(FName\(".*?"\)\))\);', 'SetActivationBlockedTags(FGameplayTagContainer($1));')
    [IO.File]::WriteAllText($f.FullName, $content)
}
