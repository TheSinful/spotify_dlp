## 0. Spotify Integration 
- [x] **Login to Spotify API using OAuth using ClientID & ClientSecret**  
- [x] **Metadata gathering** 
    - [x] Retreive metadata for tracks 
    - [x] Retreive metadata for albums
    - [x] Retreive metadata for playlists
- [x] **Testing**

## 1. YouTube Integration
- [x] **URL Generation & Video Discovery**
  - [x] Implement robust YouTube search URL builder
  - [x] Add search result parsing and validation  # Parse YouTube search results to extract video details (title, duration, views, channel) and validate against Spotify metadata
  - [x] Score/rank search results for best match  # Implement scoring algorithm based on title similarity, duration match, view count, and channel verification status
  - [ ] Handle rate limiting and errors  # Implement exponential backoff for YouTube API limits and handle network/API errors gracefully

- [x] **YT-DLP Integration**
  - [x] Extract yt-dlp binary to temp directory
  - [ ] Implement process management for yt-dlp calls  # Manage subprocess creation, IPC, timeout handling, and graceful termination
  - [ ] Handle download progress callbacks  # Create event system for download progress, speed, ETA, and file size updates
  - [ ] Support various quality options  # Implement quality selection (best, worst, specific resolution) and format handling (mp3, m4a, opus)
  - [ ] Manage temp files cleanup  # Implement automatic cleanup of partial downloads and temporary files on process exit/error

- [] **Testing Suite** (Partial)
  - [x] Test URL generation with various metadata
  - [ ] Test video matching algorithms
  - [x] Test binary extraction
  - [ ] Integration tests for full workflow

## 2. C API Development
- [ ] **Core API Design**
  - [ ] Define C-compatible data structures  # Design FFI-safe structs for metadata, download options, and progress info
  - [ ] Create error handling system  # Implement error codes, thread-local error contexts, and error propagation
  - [ ] Implement memory management functions  # Create allocation/deallocation functions with clear ownership semantics
  - [ ] Design thread-safe operations  # Ensure all operations are thread-safe using mutex/atomic operations where needed

- [ ] **Error System**
  - [ ] Define comprehensive error enum
  - [ ] Add error descriptions
  - [ ] Implement error context system
  - [ ] Create error translation utilities

- [ ] **Testing**
  - [ ] Unit tests for C API functions
  - [ ] Memory leak tests
  - [ ] Thread safety tests
  - [ ] Error handling tests

## 3. Language Bindings
- [ ] **C/C++ Integration**
  - [ ] Create vcpkg package
  - [ ] Write CMake integration
  - [ ] Document build requirements

- [ ] **Python Bindings**
  - [ ] Create PyPI package  # Package library with wheels for major platforms/Python versions
  - [ ] Implement Python native types conversion  # Convert between C types and Python objects (dict, list, str)
  - [ ] Add async support  # Implement async/await support using Python's asyncio
  - [ ] Write pip installation guide  # Document installation process, requirements, and troubleshooting

- [ ] **Node.js Bindings**
  - [ ] Create npm package
  - [ ] Implement N-API wrapper
  - [ ] Add Promise support
  - [ ] Write npm installation guide

- [ ] **Rust Bindings**
  - [ ] Create crate
  - [ ] Implement safe Rust wrapper
  - [ ] Add cargo integration
  - [ ] Write cargo installation guide

- [ ] **Java Bindings**
  - [ ] Create Maven artifact
  - [ ] Implement JNI wrapper
  - [ ] Add Gradle support
  - [ ] Write Maven/Gradle installation guide

- [ ] **C# Bindings**
  - [ ] Create NuGet package
  - [ ] Implement P/Invoke wrapper
  - [ ] Add async support
  - [ ] Write NuGet installation guide

- [ ] **Testing Infrastructure**
  - [ ] Create CI/CD pipeline for each binding
  - [ ] Cross-platform tests
  - [ ] Integration tests per language
  - [ ] Package deployment tests

## 4. Documentation
- [ ] **API Documentation**
  - [ ] Document all public APIs
  - [ ] Add method signatures
  - [ ] Include parameter descriptions
  - [ ] Document error conditions

- [ ] **Usage Guide**
  - [ ] Core library usage
  - [ ] Language-specific guides
  - [ ] Error handling examples
  - [ ] Best practices

- [ ] **Code Examples**
  - [ ] Basic usage examples
  - [ ] Advanced scenarios
  - [ ] Language-specific idioms
  - [ ] Error handling patterns

- [ ] **Build Documentation**
  - [ ] Prerequisites guide
  - [ ] Build commands
  - [ ] Troubleshooting section
  - [ ] Platform-specific notes

## 5. CLI
- [ ] **Command Line Interface**
  - [ ] Core Commands
    - [ ] Download (`spotify-dlp <url> <format>`)  # Main download command with URL detection and format selection
    - [ ] List formats (`spotify-dlp formats`)  # Show available download formats and quality options
    - [ ] Version info (`spotify-dlp --version`)  # Display version, build info, and dependency versions
  
  - [ ] Configuration
    - [ ] Config file support  # JSON/YAML config with defaults, paths, API keys
    - [ ] CLI arguments parsing  # Use clap/argparse for robust argument handling
    - [ ] Default options  # Implement override hierarchy: CLI > config file > hardcoded defaults

  - [ ] Features
    - [ ] Progress bars  # Interactive progress display with speed/ETA
    - [ ] Download statistics  # Show transfer speed, file sizes, completion %
    - [ ] Concurrent downloads  # Implement download queue with configurable parallelism
    - [ ] Resume support  # Handle partial downloads and resume capability
    - [ ] Output formatting options
    - [ ] Verbose/debug logging

  - [ ] Testing
    - [ ] Command parsing tests
    - [ ] Integration tests
    - [ ] Cross-platform testing
    - [ ] Mock filesystem tests

  - [ ] Documentation
    - [ ] Man pages
    - [ ] Help text
    - [ ] Examples
    - [ ] Configuration guide

## 6. Build System 
- [x] **CMake Configuration**
  - [x] Basic build setup
  - [x] Configure dependencies (CURL, JSON, spdlog, fmt)
  - [x] Platform-specific handling
  - [ ] Install targets  # Configure installation paths, dependencies, and platform-specific files

- [ ] **Build Testing**
  - [ ] Test on Windows/Linux/MacOS
  - [ ] Document platform differences
  - [ ] Create build matrices