cmake_minimum_required(VERSION 3.14)
project(texture17 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ソースファイル
set(SOURCES
    main.cpp
    box.cpp
    trackball.cpp
)

# ヘッダファイル
set(HEADERS
    box.h
    trackball.h
)

# リソースファイル
set(RESOURCE_FILES
    "dot.raw"
    "room2nx.raw"
    "room2ny.raw"
    "room2nz.raw"
    "room2px.raw"
    "room2py.raw"
    "room2pz.raw"
)

if(APPLE)

    # macOS の設定
    set(ICON_FILE "opengl.icns")

    # アプリケーションバンドルとして実行ファイルを作成する
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${RESOURCE_FILES} ${ICON_FILE})

    # GLUT と OpenGL の Framework を検索してリンクする
    find_library(GLUT_LIBRARY GLUT)
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLUT_LIBRARY} ${OPENGL_LIBRARY})

    # アプリケーションバンドルの作成設定
    set_target_properties(${PROJECT_NAME} PROPERTIES

        # アプリケーションバンドルとしてビルドする
        MACOSX_BUNDLE TRUE

        # Info.plist に書き込む内容
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_ICON_FILE "${ICON_FILE}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 Kohe Tokoi. All rights reserved."

        # Xcode のスキームを生成する
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # リソースの配置設定
    set_source_files_properties(${RESOURCE_FILES} ${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # リソースファイルなどをアプリケーションバンドル内の Resources に組み込む
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # Xcode 上で Run したときに起動するようにアプリケーションバンドルをスタートアッププロジェクトにする
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY XCODE_GENERATE_SCHEME ON)

elseif(WIN32)

    # Windows (Visual Studio) の設定
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    # glext.h と khrplatform.h をダウンロードする
    include(FetchContent)
    set(GLEXT_H "${CMAKE_BINARY_DIR}/include/GL/glext.h")
    set(KHRPLATFORM_H "${CMAKE_BINARY_DIR}/include/KHR/khrplatform.h")
    if(NOT EXISTS "${GLEXT_H}")
        file(DOWNLOAD "https://www.khronos.org/registry/OpenGL/api/GL/glext.h" "${GLEXT_H}")
    endif()
    if(NOT EXISTS "${KHRPLATFORM_H}")
        file(DOWNLOAD "https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h" "${KHRPLATFORM_H}")
    endif()

    # glext.h と khrplatform.h をインクルードパスに追加する
    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/include")

    # freeglut のソースを取得してビルドする
    FetchContent_Declare(
        freeglut
        GIT_REPOSITORY https://github.com/FreeGLUTProject/freeglut.git
        GIT_TAG master
    )

    # freeglut の不要な共有ライブラリやデモをビルドしない
    set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(freeglut)

    # OpenGL をリンクする
    find_package(OpenGL REQUIRED)

    # freeglut のヘッダファイルのパスを通す
    target_include_directories(${PROJECT_NAME} PRIVATE "${freeglut_SOURCE_DIR}/include")

    # freeglut は STATIC リンクする
    target_link_libraries(${PROJECT_NAME} PRIVATE freeglut_static OpenGL::GL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FREEGLUT_STATIC)

    # 起動時に読み込めるようにビルド後にリソースファイルを実行ファイルと同じ階層にコピーする
    foreach(TEX_FILE IN LISTS RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${TEX_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()

    # このプロジェクトを Visual Studio のスタートアッププロジェクトとして設定する
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # Visual Studio で実行する際の作業ディレクトリを実行ファイルと同階層にする
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

else()

    # Ubuntu Linux (その他UNIX系) の設定
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    # OpenGL と GLUT を探す
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)

    # freeglut (glut) と gl と数学ライブラリ m をリンクする
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        OpenGL::GL
        OpenGL::GLU
        GLUT::GLUT
        m
    )

    # 起動時に読み込めるようにビルド後にリソースファイルを実行ファイルと同じ階層にコピーする
    foreach(TEX_FILE IN LISTS RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${TEX_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()

endif()
