# Create a custom library
add_library(
  onyxComponents
  lib/onyxComponents.cc
)

# Apps here ...
add_library(
  onyxClientApp
  lib/onyxClientApp.cc
)

add_library(
  onyxProxyApp
  lib/onyxProxyApp.cc
)

add_library(
  onyxReceiverApp
  lib/onyxReceiverApp.cc
)

# Link the custom library to the scratch executable
build_exec(
  EXECNAME onyxSimulation
  SOURCE_FILES onyxSimulation.cc
  LIBRARIES_TO_LINK onyxComponents
                    onyxClientApp
                    onyxProxyApp
                    onyxReceiverApp
                    ${libcore}
                    ${libpoint-to-point}
                    ${libinternet}
                    ${libapplications}
  # use "${ns3-libs}" "${ns3-contrib-libs}" in case you want to link to all
  # enabled modules
  EXECUTABLE_DIRECTORY_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
