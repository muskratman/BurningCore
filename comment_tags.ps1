$files = Get-ChildItem -Path "c:\UE_projects\BurningCORE\Source\BurningCORE\GAS\Abilities\GA_*.cpp"
foreach ($f in $files) {
    (Get-Content $f.FullName) -replace 'SetAssetTags', '// SetAssetTags' -replace 'SetActivationOwnedTags', '// SetActivationOwnedTags' -replace 'SetActivationBlockedTags', '// SetActivationBlockedTags' | Set-Content $f.FullName
}
