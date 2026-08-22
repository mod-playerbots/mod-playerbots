#
# This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
# information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
# or (at your option) any later version.
#

# Gates the module's hooks in core headers and in the database layer.
target_compile_options(database
  PRIVATE
    -DMOD_PLAYERBOTS)

target_compile_options(game-interface
  INTERFACE
    -DMOD_PLAYERBOTS)

target_compile_options(modules
  PRIVATE
    -DMOD_PLAYERBOTS)

# database links mysql PRIVATE, so the mysql headers are not on the modules include path.
# DatabaseEnv.h and PlayerbotsDatabase.h pull them in.
target_link_libraries(modules
  PRIVATE
    mysql)

# boost::thread is used by PlayerbotCommandServer and RandomPlayerbotMgr. deps/boost already
# validated the minimum boost version, so no version is requested here.
find_package(Boost REQUIRED COMPONENTS thread)

target_link_libraries(modules
  PRIVATE
    Boost::thread)
