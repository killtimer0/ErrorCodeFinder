#include "../ecf.h"

#define Ok                          0
#define GenericError                1
#define InvalidParameter            2
#define OutOfMemory                 3
#define ObjectBusy                  4
#define InsufficientBuffer          5
#define NotImplemented              6
#define Win32Error                  7
#define WrongState                  8
#define Aborted                     9
#define FileNotFound                10
#define ValueOverflow               11
#define AccessDenied                12
#define UnknownImageFormat          13
#define FontFamilyNotFound          14
#define FontStyleNotFound           15
#define NotTrueTypeFont             16
#define UnsupportedGdiplusVersion   17
#define GdiplusNotInitialized       18
#define PropertyNotFound            19
#define PropertyNotSupported        20
#define ProfileNotFound             21

static ErrorCodeItem ec_gdip_items[] = {
    REGCODE(Ok)
    REGCODE(GenericError)
    REGCODE(InvalidParameter)
    REGCODE(OutOfMemory)
    REGCODE(ObjectBusy)
    REGCODE(InsufficientBuffer)
    REGCODE(NotImplemented)
    REGCODE(Win32Error)
    REGCODE(WrongState)
    REGCODE(Aborted)
    REGCODE(FileNotFound)
    REGCODE(ValueOverflow)
    REGCODE(AccessDenied)
    REGCODE(UnknownImageFormat)
    REGCODE(FontFamilyNotFound)
    REGCODE(FontStyleNotFound)
    REGCODE(NotTrueTypeFont)
    REGCODE(UnsupportedGdiplusVersion)
    REGCODE(GdiplusNotInitialized)
    REGCODE(PropertyNotFound)
    REGCODE(PropertyNotSupported)
    REGCODE(ProfileNotFound)
};

static LPTSTR gdip_code2msg(DWORD dwCode)
{
    switch (dwCode)
    {
        case Ok:
        return TEXT("The method call was successful.");

        case GenericError:
        return TEXT("There was an error on the method call, which is identified ")
               TEXT("as something other than those defined by the other elements ")
               TEXT("of this enumeration.");

        case InvalidParameter:
        return TEXT("One of the arguments passed to the method was not valid.");

        case OutOfMemory:
        return TEXT("The operating system is out of memory and could not ")
               TEXT("allocate memory to process the method call.");

        case ObjectBusy:
        return TEXT("One of the arguments specified in the API call is already ")
               TEXT("in use in another thread.");

        case InsufficientBuffer:
        return TEXT("A buffer specified as an argument in the API call ")
               TEXT("is not large enough to hold the data to be received.");

        case NotImplemented:
        return TEXT("The method is not implemented.");

        case Win32Error:
        return TEXT("The method generated a Win32 error.");

        case WrongState:
        return TEXT("The object is in an invalid state to satisfy the API call.");

        case Aborted:
        return TEXT("The method was aborted.");

        case FileNotFound:
        return TEXT("The specified image file or metafile cannot be found.");

        case ValueOverflow:
        return TEXT("The method performed an arithmetic operation ")
               TEXT("that produced a numeric overflow.");

        case AccessDenied:
        return TEXT("A write operation is not allowed on the specified file.");

        case UnknownImageFormat:
        return TEXT("The specified image file format is not known.");

        case FontFamilyNotFound:
        return TEXT("The specified font family cannot be found.");

        case FontStyleNotFound:
        return TEXT("The specified style is not available for the ")
               TEXT("specified font family.");

        case NotTrueTypeFont:
        return TEXT("The font retrieved from an HDC or LOGFONT ")
               TEXT("is not a TrueType font and cannot be used with GDI+.");

        case UnsupportedGdiplusVersion:
        return TEXT("The version of GDI+ that is installed on the system ")
               TEXT("is incompatible with the version with ")
               TEXT("which the application was compiled.");

        case GdiplusNotInitialized:
        return TEXT("The GDI+ API is not in an initialized state.");

        case PropertyNotFound:
        return TEXT("The specified property does not exist in the image.");

        case PropertyNotSupported:
        return TEXT("The specified property is not supported by the format ")
               TEXT("of the image and, therefore, cannot be set.");

        case ProfileNotFound:
        return TEXT("The color profile required to save an image ")
               TEXT("in CMYK format was not found.");

        default:
        return NULL;
    }
}

static void gdip_freemsg(LPTSTR lpMessage)
{
    /* Ignore */
}

void EcRegGdip(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("Gdiplus");
    pecs->pfnMessageFromCode = gdip_code2msg;
    pecs->pfnFreeMessage = gdip_freemsg;
    pecs->items = ec_gdip_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_gdip_items);
}

void EcUnregGdip()
{
    /* Ignore */
}

