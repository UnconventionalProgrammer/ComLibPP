set(WinComLibPP_INC
        ComLibPP/ComLibPP.hpp
        ComlibPP/export.hpp
        ComLibPP/ISerialDriver.hpp
        ComLibPP/Win32SerialDriver.hpp
)

PREPEND(WinComLibPP_INC)
set(WinComLibPP_INC ${WinComLibPP_INC}  PARENT_SCOPE)