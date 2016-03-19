
if (MINGW)
    set(_RC_FILE vCardTool_mingw.rc)
else()
    set(_RC_FILE vCardTool.rc)
endif()

add_executable(vCardTool WIN32
    common.cpp
    common.h
    DlgAbout.cpp
    DlgAbout.h
    RegHelper.cpp
    RegHelper.h
    resource.h
    Tim/ClassEx.h
    Tim/CommonCtrls.h
    Tim/Dialog.cpp
    Tim/Dialog.h
    Tim/File.cpp
    Tim/File.h
    Tim/Http.cpp
    Tim/Http.h
    Tim/LinkCtrl.cpp
    Tim/LinkCtrl.h
    Tim/Rect.h
    Tim/SException.cpp
    Tim/SException.h
    Tim/tim.h
    Tim/TString.cpp
    Tim/TString.h
    Tim/Window.h
    vCard.cpp
    vCard.h
    ${_RC_FILE}
    vCardWnd.cpp
    vCardWnd.h
    WinMain.cpp
    )

target_link_libraries(vCardTool
    shlwapi
    wininet
    comctl32
    )
