$filePath = ".\VideohubSimulator\debug\BmdVideoHub.exe"
$zipName = "VideohubSimulator_v1.0.1_debug.zip"
$url = "https://github.com/thedmo/VideoHub-Simulator/releases/download/v1.0.1/debug.zip"

if (Test-Path -Path $filePath) {
    Write-Host "Simulator already downloaded..."
}
else {
    Write-Host "Creating directory..."
    New-Item -ItemType Directory -Path $filePath | Out-Null

    Write-Host "Downloading Videohub Simulator..."
    Invoke-WebRequest -Uri $url -OutFile $zipName
    
    Write-Host "Unzipping archive..."
    Expand-Archive -Path ".\$zipName" -DestinationPath $filePath
    
    Write-Host "Cleaning up..."
    Remove-Item $zipName
}
