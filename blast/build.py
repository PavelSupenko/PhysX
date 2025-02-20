#!/usr/bin/env python3

import os
import sys
import shutil
import subprocess
import platform
from pathlib import Path

def clear_build_directory(build_dir):
    """
    Removes the existing build directory to clear CMake cache.
    """
    if os.path.exists(build_dir):
        print(f"Removing existing build directory: {build_dir}")
        try:
            shutil.rmtree(build_dir)
        except Exception as e:
            print(f"Error removing build directory: {e}")
            sys.exit(1)
    else:
        print(f"No existing build directory found at: {build_dir}")

def create_build_directory(build_dir):
    """
    Creates a fresh build directory.
    """
    try:
        os.makedirs(build_dir, exist_ok=True)
        print(f"Created build directory: {build_dir}")
    except Exception as e:
        print(f"Error creating build directory: {e}")
        sys.exit(1)

def run_cmake_configure(cmake_command, build_dir):
    """
    Runs the CMake configure command and logs output.
    """
    log_file = os.path.join(build_dir, "cmake_configure.log")
    print("Configuring CMake...")
    with open(log_file, "w") as log:
        try:
            subprocess.check_call(cmake_command, stdout=log, stderr=log)
            print(f"CMake configuration succeeded. Logs are in {log_file}")
        except subprocess.CalledProcessError as e:
            print(f"CMake configuration failed. Check the log at {log_file} for details.")
            sys.exit(1)

def run_cmake_build(cmake_build_command, build_dir):
    """
    Runs the CMake build command and logs output.
    """
    log_file = os.path.join(build_dir, "cmake_build.log")
    print("Building the project...")
    with open(log_file, "w") as log:
        try:
            subprocess.check_call(cmake_build_command, stdout=log, stderr=log)
            print(f"Build succeeded. Logs are in {log_file}")
        except subprocess.CalledProcessError as e:
            print(f"Build failed. Check the log at {log_file} for details.")
            sys.exit(1)

def get_host_os():
    """
    Detects the host operating system.
    Returns 'Windows', 'Darwin' (macOS), or 'Linux'.
    """
    return platform.system()

def get_build_targets(host_os):
    """
    Returns a list of build targets based on the host OS.
    """
    if host_os == "Windows":
        return [
            "windows-x86",
            "windows-x86_64",
            "android-armv7",
            "android-armv8",
            "android-x86",
            "android-x86_64"
        ]
    elif host_os == "Darwin":
        return [
            "macos-x86_64",
            "macos-arm64",
            "ios-arm64",
            "android-armv7",
            "android-armv8",
            "android-x86",
            "android-x86_64"
        ]
    else:
        print(f"Unsupported host OS: {host_os}")
        sys.exit(1)

def prompt_user_selection(options, prompt_message):
    """
    Displays a list of options and prompts the user to select one.
    Returns the selected option.
    """
    print(prompt_message)
    for idx, option in enumerate(options, start=1):
        print(f"  {idx}. {option}")
    
    while True:
        try:
            selection = int(input(f"Enter the number of your choice (1-{len(options)}): "))
            if 1 <= selection <= len(options):
                return options[selection - 1]
            else:
                print(f"Please enter a number between 1 and {len(options)}.")
        except ValueError:
            print("Invalid input. Please enter a valid number.")

def check_cmake_installed():
    """
    Checks if CMake is installed and accessible.
    """
    try:
        subprocess.check_call(["cmake", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    except FileNotFoundError:
        print("Error: CMake is not installed or not found in PATH.")
        sys.exit(1)
    except subprocess.CalledProcessError:
        print("Error: CMake is installed but not functioning correctly.")
        sys.exit(1)

def prompt_vs_generator():
    """
    Prompts the Windows user to specify a CMake generator or use the default.
    Returns the generator string or None if default should be used.
    """
    print("\nWould you like to specify a CMake generator for Visual Studio?")
    print("  1. Yes")
    print("  2. No (Use default generator)")
    while True:
        choice = input("Enter your choice (1-2): ").strip()
        if choice == "1":
            # Display a list of common Visual Studio generators
            generators = [
                "Visual Studio 16 2019",
                "Visual Studio 17 2022",
                "Visual Studio 15 2017",
                # Add more as needed
            ]
            print("\nAvailable Visual Studio Generators:")
            for idx, gen in enumerate(generators, start=1):
                print(f"  {idx}. {gen}")
            while True:
                try:
                    gen_choice = int(input(f"Select the generator number (1-{len(generators)}): "))
                    if 1 <= gen_choice <= len(generators):
                        return generators[gen_choice - 1]
                    else:
                        print(f"Please enter a number between 1 and {len(generators)}.")
                except ValueError:
                    print("Invalid input. Please enter a valid number.")
        elif choice == "2":
            print("Proceeding with the default CMake generator.")
            return None
        else:
            print("Invalid choice. Please enter 1 or 2.")

def main():
    # Check if CMake is installed
    check_cmake_installed()

    host_os = get_host_os()
    print(f"Detected host OS: {host_os}")

    build_targets = get_build_targets(host_os)
    target_platform = prompt_user_selection(build_targets, "Available Build Targets:")

    # Prompt for build configuration
    build_configs = ["Debug", "Release"]
    build_config = prompt_user_selection(build_configs, "Select Build Configuration:")

    # If Android target, prompt for NDK path
    android_targets = [t for t in build_targets if t.startswith("android-")]
    if target_platform in android_targets:
        while True:
            ndk_path = input("Enter the path to the Android NDK: ").strip()
            if os.path.isdir(ndk_path):
                # Verify that the toolchain file exists
                toolchain_file = os.path.join(ndk_path, "build", "cmake", "android.toolchain.cmake")
                if os.path.isfile(toolchain_file):
                    break
                else:
                    print(f"Error: CMake toolchain file not found at {toolchain_file}. Please verify the NDK path.")
            else:
                print("Invalid NDK path. Please enter a valid directory path.")
    else:
        ndk_path = None

    # Define default output directory
    default_output_dir = "build_artifacts"
    output_dir = os.path.abspath(default_output_dir)
    print(f"\nUsing default output directory: {output_dir}")
    build_dir = os.path.join(output_dir, "build", target_platform)

    # Clear and create build directory
    clear_build_directory(build_dir)
    create_build_directory(build_dir)

    # Define CMake generator and flags based on target platform
    cmake_generator = ""
    cmake_flags = [
        f"-DCMAKE_BUILD_TYPE={build_config}",
        f"-DNV_CONFIGURATION_TYPE={build_config}"
    ]

    # Determine the source directory based on the host OS
    if host_os == "Windows":
        source_dir = ".\\"
    else:
        source_dir = "./"

    if target_platform.startswith("windows-"):
        arch = target_platform.split("-")[-1]
        if arch == "x86":
            cmake_flags.append("-A Win32")
        elif arch == "x86_64":
            cmake_flags.append("-A x64")
        else:
            print(f"Unsupported Windows architecture: {arch}")
            sys.exit(1)
        
        # Prompt Windows user to specify Visual Studio generator or use default
        generator = prompt_vs_generator()
        if generator:
            cmake_generator = generator
            print(f"Selected CMake generator: {cmake_generator}")
        else:
            # Do not pass the -G option, allowing CMake to choose the default
            print("CMake will use the default generator.")

    elif target_platform.startswith("macos-"):
        cmake_generator = "Xcode"
        arch = target_platform.split("-")[-1]
        if arch in ["x86_64", "arm64"]:
            cmake_flags.append(f"-DCMAKE_OSX_ARCHITECTURES={arch}")
        else:
            print(f"Unsupported macOS architecture: {arch}")
            sys.exit(1)

    elif target_platform.startswith("ios-"):
        cmake_generator = "Xcode"
        arch = target_platform.split("-")[-1]
        if arch == "arm64":
            cmake_flags.extend([
                "-DCMAKE_SYSTEM_NAME=iOS",
                "-DCMAKE_OSX_SYSROOT=iphoneos",
                "-DCMAKE_OSX_ARCHITECTURES=arm64"
            ])
        else:
            print(f"Unsupported iOS architecture: {arch}")
            sys.exit(1)

    elif target_platform.startswith("android-"):
        cmake_generator = "Unix Makefiles"
        abi = ""
        platform_api = "android-21"  # Adjust as needed

        # Map build target to ANDROID_ABI
        if target_platform == "android-armv7":
            abi = "armeabi-v7a"
        elif target_platform == "android-armv8":
            abi = "arm64-v8a"
        elif target_platform == "android-x86":
            abi = "x86"
        elif target_platform == "android-x86_64":
            abi = "x86_64"
        else:
            print(f"Unsupported Android target: {target_platform}")
            sys.exit(1)

        cmake_flags.extend([
            f"-DCMAKE_TOOLCHAIN_FILE={ndk_path}/build/cmake/android.toolchain.cmake",
            f"-DANDROID_ABI={abi}",
            f"-DANDROID_PLATFORM={platform_api}"
        ])
    else:
        print(f"Unsupported target platform: {target_platform}")
        sys.exit(1)

    # Construct CMake configure command
    cmake_command = [
        "cmake",
        source_dir,
        "-B", build_dir
    ]

    # Append CMake generator if specified (Windows)
    if target_platform.startswith("windows-") and cmake_generator:
        cmake_command.extend(["-G", cmake_generator])

    # Append CMake flags
    cmake_command.extend(cmake_flags)

    # Display the CMake command for transparency
    print("\nCMake Configuration Command:")
    print(" ".join(cmake_command))

    # Run CMake configure
    run_cmake_configure(cmake_command, build_dir)

    # Construct CMake build command
    cmake_build_command = [
        "cmake",
        "--build", build_dir,
        "--config", build_config
    ]

    # For multi-threaded builds on Unix-like systems (macOS and Android)
    if host_os in ["Linux", "Darwin"] and not target_platform.startswith("windows-"):
        cpu_count = os.cpu_count() or 1
        cmake_build_command += ["--", f"-j{cpu_count}"]

    # Display the CMake build command for transparency
    print("\nCMake Build Command:")
    print(" ".join(cmake_build_command))

    # Run CMake build
    run_cmake_build(cmake_build_command, build_dir)

    print(f"\nBuild succeeded for {target_platform}. Artifacts are located in {build_dir}.")
    print(f"All builds completed successfully. Artifacts are in '{output_dir}'.")

if __name__ == "__main__":
    main()
