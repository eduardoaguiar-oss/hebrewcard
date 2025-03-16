cmake_minimum_required(VERSION 3.10)
project(HebrewTrainer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ALLEGRO_INCLUDE_DIRS "/usr/include/allegro5")  # Update this path if necessary
set(ALLEGRO_LIBRARIES
    allegro
    allegro_acodec
    allegro_audio
    allegro_color
    allegro_font
    allegro_image
    allegro_ttf
    allegro_primitives
)

set(ALLEGROPP_LIBRARIES allegropp)

# Find Allegro and its components
#find_package(Allegro REQUIRED COMPONENTS main font ttf primitives)

# Add the executable
add_executable(hebrewcard
    hebrewcard.cpp
)

# Link Allegro libraries
#3target_link_libraries(hebrewcard
#    PRIVATE
#    Allegro::Allegro
#    Allegro::Allegro_font
#    Allegro::Allegro_ttf
#    Allegro::Allegro_primitives
#)

option(ALLEGROPP_USE_DEV "Use the development version of allegropp" ON)

if(ALLEGROPP_USE_DEV)
    # Development mode: Use add_subdirectory() to include the allegropp project
    message(STATUS "Using development version of allegropp")
    add_subdirectory(../allegropp allegropp)
    #set(ALLEGROPP_INCLUDE_DIRS "../allegropp/src")

else()
    # Production mode: Use find_package() to locate the installed allegropp
    message(STATUS "Using installed version of allegropp")
    find_package(allegropp REQUIRED)
endif()

# Include Allegro headers
target_include_directories(hebrewcard PRIVATE ${ALLEGRO_INCLUDE_DIRS})

# Link Allegro libraries
target_link_libraries(hebrewcard PRIVATE ${ALLEGRO_LIBRARIES} allegropp)

# Install the executable (optional)
install(TARGETS hebrewcard DESTINATION bin)
