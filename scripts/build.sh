#!/bin/bash

# Dungeon Merc Build Script
# This script provides an easy way to build the project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check prerequisites
check_prerequisites() {
    print_status "Checking prerequisites..."

    if ! command_exists cmake; then
        print_error "CMake is not installed. Please install CMake 3.16 or higher."
        exit 1
    fi

    if ! command_exists make; then
        print_error "Make is not installed. Please install Make."
        exit 1
    fi

    if ! command_exists g++; then
        print_error "G++ compiler is not installed. Please install a C++17 compatible compiler."
        exit 1
    fi

    print_success "All prerequisites are satisfied"
}

# Function to clean build directory
clean_build() {
    print_status "Cleaning build directory..."
    rm -rf build/
    print_success "Build directory cleaned"
}

# Function to build with CMake
build_cmake() {
    print_status "Building with CMake..."

    # Create build directory
    mkdir -p build
    cd build

    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release

    # Build
    print_status "Compiling..."
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    cd ..
    print_success "Build completed successfully"
}

# Function to build with Makefile
build_makefile() {
    print_status "Building with Makefile..."
    make clean
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    print_success "Build completed successfully"
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    if [ -f "build/dungeon_merc_tests" ]; then
        ./build/dungeon_merc_tests
        print_success "Tests completed"
    else
        print_warning "Test executable not found. Run build first."
    fi
}

# Function to install
install_binary() {
    print_status "Installing binary..."
    if [ -f "bin/dungeon_merc" ]; then
        sudo cp bin/dungeon_merc /usr/local/bin/
        print_success "Binary installed to /usr/local/bin/dungeon_merc"
    else
        print_error "Binary not found. Run build first."
        exit 1
    fi
}

# Function to show help
show_help() {
    echo "Dungeon Merc Build Script"
    echo ""
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build          Build the project (default)"
    echo "  clean          Clean build directory"
    echo "  test           Run tests"
    echo "  install        Install binary to /usr/local/bin"
    echo "  all            Clean, build, test, and install"
    echo "  help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0              # Build the project"
    echo "  $0 clean        # Clean build directory"
    echo "  $0 test         # Run tests"
    echo "  $0 all          # Complete build process"
}

# Main script logic
main() {
    case "${1:-build}" in
        "build")
            check_prerequisites
            build_cmake
            ;;
        "clean")
            clean_build
            ;;
        "test")
            run_tests
            ;;
        "install")
            install_binary
            ;;
        "all")
            check_prerequisites
            clean_build
            build_cmake
            run_tests
            install_binary
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
