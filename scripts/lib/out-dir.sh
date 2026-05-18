#!/bin/bash
# Shared output directory helpers. Source from build scripts:
#   source "$(leanclr_scripts_lib)/out-dir.sh"

leanclr_scripts_lib="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

leanclr_repo_root() {
    cd "$leanclr_scripts_lib/../.." && pwd
}

leanclr_out_root() {
    if [[ -n "${LEANCLR_OUT_ROOT:-}" ]]; then
        echo "$LEANCLR_OUT_ROOT"
    else
        echo "$(leanclr_repo_root)/out"
    fi
}

# Usage: leanclr_cmake_build_dir <module/path> <config> [arch]
leanclr_cmake_build_dir() {
    local module="$1"
    local config="$2"
    local arch="${3:-}"
    local root
    root="$(leanclr_out_root)"
    if [[ -n "$arch" ]]; then
        echo "$root/cmake/$module/${config}-${arch}"
    else
        echo "$root/cmake/$module/${config}"
    fi
}

# Usage: leanclr_dotnet_out_dir <ProjectName> [Configuration]
leanclr_dotnet_out_dir() {
    local project="$1"
    local config="${2:-Debug}"
    echo "$(leanclr_out_root)/dotnet/$project/$config"
}

# Usage: leanclr_dotnet_sdk_exe <ProjectName> <Configuration> <TargetFramework>
leanclr_dotnet_sdk_exe() {
    local project="$1"
    local config="${2:-Debug}"
    local tfm="${3:-net8.0}"
    echo "$(leanclr_out_root)/dotnet/$project/$config/$tfm/$project"
}
