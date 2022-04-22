param([switch] $Force, [switch] $DryRun)

Get-ChildItem tests | ForEach-Object {
    $BasePath = $_
    if ($Force) {
        Remove-Item $BasePath/*.comp.spv
    }

    $PassArgs = (Get-Content "$BasePath/__pass__" | ForEach-Object { return $_.Trim(); } | Where-Object { $_ -ne "" } | ForEach-Object { return  "-p "+ $_; }) -join ' '

    Get-ChildItem $BasePath/*.comp | ForEach-Object {
        # Compile test input shaders first.
        $InPath = $_;
        $OutPath = "$_.spv";
        if (-not (Test-Path $OutPath)) {
            & glslangValidator $InPath -o $OutPath -V
        }

        Write-Host "testing " + $OutPath
        $DbgPrintPath = $OutPath + ".log";
        if ($DryRun) {
            Write-Host "./out/build/x64-Debug/bin/cspv.exe -i $OutPath --dbg-print-file $DbgPrintPath $PassArgs"
        } else {
            Invoke-Expression "./out/build/x64-Debug/bin/cspv.exe -i $OutPath --dbg-print-file $DbgPrintPath $PassArgs"
        }
    }
}
