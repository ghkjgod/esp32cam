# Test build script
Write-Host "Testing ESP-IDF environment..."

# Set environment variables
$env:IDF_PATH = "d:\workspace\esp\v5.5\esp-idf"
$env:IDF_TOOLS_PATH = "d:\workspace\idftools"
$env:PYTHON = "d:\workspace\idftools\python_env\idf5.5_py3.11_env\Scripts\python.exe"
$env:PYTHONPATH = "d:\workspace\esp\v5.5\esp-idf\tools"

# Test if we can find cmake
$cmake_path = "d:\workspace\idftools\tools\cmake\3.30.2\bin\cmake.exe"
if (Test-Path $cmake_path) {
    Write-Host "CMake found at: $cmake_path"
} else {
    Write-Host "CMake not found!"
    exit 1
}

# Test if we can find ninja
$ninja_path = "d:\workspace\idftools\tools\ninja\1.12.1\ninja.exe"
if (Test-Path $ninja_path) {
    Write-Host "Ninja found at: $ninja_path"
} else {
    Write-Host "Ninja not found!"
    exit 1
}

# Test if we can find compiler
$gcc_path = "d:\workspace\idftools\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin\xtensa-esp32s3-elf-gcc.exe"
if (Test-Path $gcc_path) {
    Write-Host "GCC found at: $gcc_path"
} else {
    Write-Host "GCC not found!"
    exit 1
}

# Test if we can find the correct Python
$python_path = "d:\workspace\idftools\python_env\idf5.5_py3.11_env\Scripts\python.exe"
if (Test-Path $python_path) {
    Write-Host "Python found at: $python_path"
} else {
    Write-Host "Python not found!"
    exit 1
}

Write-Host "All tools found. Attempting to configure project..."

# Add tools to PATH
$env:PATH = "d:\workspace\idftools\tools\ninja\1.12.1;d:\workspace\idftools\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;$env:PATH"

# Try to configure the project directly with cmake
& $cmake_path -G "Ninja" -DPYTHON_DEPS_CHECKED=1 -DESP_PLATFORM=1 -DIDF_TARGET=esp32s3 -B "build" -S "."

if ($LASTEXITCODE -eq 0) {
    Write-Host "Configuration successful!"
    Write-Host "Attempting to build project..."
    & $cmake_path --build "build"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build successful!"
    } else {
        Write-Host "Build failed with exit code: $LASTEXITCODE"
    }
} else {
    Write-Host "Configuration failed with exit code: $LASTEXITCODE"
}
