#include "../ecf.h"
#include <d3d9.h>

#include <ddraw.h>

/* http://tdxlibrary.org/TDx_Library/TDx_3DI_Library/Glossary/Glossary_*.htm */

#define D3DERR_BADMAJORVERSION          MAKE_DDHRESULT(700)
#define D3DERR_BADMINORVERSION          MAKE_DDHRESULT(701)
#define D3DERR_INVALID_DEVICE           MAKE_DDHRESULT(705)
#define D3DERR_INITFAILED               MAKE_DDHRESULT(706)
#define D3DERR_DEVICEAGGREGATED         MAKE_DDHRESULT(707)
#define D3DERR_EXECUTE_CREATE_FAILED    MAKE_DDHRESULT(710)
#define D3DERR_EXECUTE_DESTROY_FAILED   MAKE_DDHRESULT(711)
#define D3DERR_EXECUTE_LOCK_FAILED      MAKE_DDHRESULT(712)
#define D3DERR_EXECUTE_UNLOCK_FAILED    MAKE_DDHRESULT(713)
#define D3DERR_EXECUTE_LOCKED           MAKE_DDHRESULT(714)
#define D3DERR_EXECUTE_NOT_LOCKED       MAKE_DDHRESULT(715)
#define D3DERR_EXECUTE_FAILED           MAKE_DDHRESULT(716)
#define D3DERR_EXECUTE_CLIPPED_FAILED   MAKE_DDHRESULT(717)
#define D3DERR_TEXTURE_NO_SUPPORT       MAKE_DDHRESULT(720)
#define D3DERR_TEXTURE_CREATE_FAILED    MAKE_DDHRESULT(721)
#define D3DERR_TEXTURE_DESTROY_FAILED   MAKE_DDHRESULT(722)
#define D3DERR_TEXTURE_LOCK_FAILED      MAKE_DDHRESULT(723)
#define D3DERR_TEXTURE_UNLOCK_FAILED    MAKE_DDHRESULT(724)
#define D3DERR_TEXTURE_LOAD_FAILED      MAKE_DDHRESULT(725)
#define D3DERR_TEXTURE_SWAP_FAILED      MAKE_DDHRESULT(726)
#define D3DERR_TEXTURE_LOCKED           MAKE_DDHRESULT(727)
#define D3DERR_TEXTURE_NOT_LOCKED       MAKE_DDHRESULT(728)
#define D3DERR_TEXTURE_GETSURF_FAILED   MAKE_DDHRESULT(729)
#define D3DERR_MATRIX_CREATE_FAILED     MAKE_DDHRESULT(730)
#define D3DERR_MATRIX_DESTROY_FAILED    MAKE_DDHRESULT(731)
#define D3DERR_MATRIX_SETDATA_FAILED    MAKE_DDHRESULT(732)
#define D3DERR_MATRIX_GETDATA_FAILED    MAKE_DDHRESULT(733)
#define D3DERR_SETVIEWPORTDATA_FAILED   MAKE_DDHRESULT(734)
#define D3DERR_INVALIDCURRENTVIEWPORT   MAKE_DDHRESULT(735)
#define D3DERR_INVALIDPRIMITIVETYPE     MAKE_DDHRESULT(736)
#define D3DERR_INVALIDVERTEXTYPE        MAKE_DDHRESULT(737)
#define D3DERR_TEXTURE_BADSIZE          MAKE_DDHRESULT(738)
#define D3DERR_INVALIDRAMPTEXTURE       MAKE_DDHRESULT(739)
#define D3DERR_MATERIAL_CREATE_FAILED   MAKE_DDHRESULT(740)
#define D3DERR_MATERIAL_DESTROY_FAILED  MAKE_DDHRESULT(741)
#define D3DERR_MATERIAL_SETDATA_FAILED  MAKE_DDHRESULT(742)
#define D3DERR_MATERIAL_GETDATA_FAILED  MAKE_DDHRESULT(743)
#define D3DERR_INVALIDPALETTE           MAKE_DDHRESULT(744)
#define D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY MAKE_DDHRESULT(745)
#define D3DERR_ZBUFF_NEEDS_VIDEOMEMORY  MAKE_DDHRESULT(746)
#define D3DERR_SURFACENOTINVIDMEM       MAKE_DDHRESULT(747)
#define D3DERR_LIGHT_SET_FAILED         MAKE_DDHRESULT(750)
#define D3DERR_LIGHTHASVIEWPORT         MAKE_DDHRESULT(751)
#define D3DERR_LIGHTNOTINTHISVIEWPORT   MAKE_DDHRESULT(752)
#define D3DERR_SCENE_IN_SCENE           MAKE_DDHRESULT(760)
#define D3DERR_SCENE_NOT_IN_SCENE       MAKE_DDHRESULT(761)
#define D3DERR_SCENE_BEGIN_FAILED       MAKE_DDHRESULT(762)
#define D3DERR_SCENE_END_FAILED         MAKE_DDHRESULT(763)
#define D3DERR_INBEGIN                  MAKE_DDHRESULT(770)
#define D3DERR_NOTINBEGIN               MAKE_DDHRESULT(771)
#define D3DERR_NOVIEWPORTS              MAKE_DDHRESULT(772)
#define D3DERR_VIEWPORTDATANOTSET       MAKE_DDHRESULT(773)
#define D3DERR_VIEWPORTHASNODEVICE      MAKE_DDHRESULT(774)
#define D3DERR_NOCURRENTVIEWPORT        MAKE_DDHRESULT(775)
#define D3DERR_INVALIDVERTEXFORMAT	    MAKE_DDHRESULT(2048)
#define D3DERR_COLORKEYATTACHED         MAKE_DDHRESULT(2050)
#define D3DERR_VERTEXBUFFEROPTIMIZED	MAKE_DDHRESULT(2060)
#define D3DERR_VBUF_CREATE_FAILED	    MAKE_DDHRESULT(2061)
#define D3DERR_VERTEXBUFFERLOCKED	    MAKE_DDHRESULT(2062)
#define D3DERR_VERTEXBUFFERUNLOCKFAILED	MAKE_DDHRESULT(2063)
#define D3DERR_ZBUFFER_NOTPRESENT	    MAKE_DDHRESULT(2070)
#define D3DERR_STENCILBUFFER_NOTPRESENT	MAKE_DDHRESULT(2071)
#define D3DERR_TOOMANYPRIMITIVES		MAKE_DDHRESULT(2083)
#define D3DERR_INVALIDMATRIX			MAKE_DDHRESULT(2084)
#define D3DERR_TOOMANYVERTICES			MAKE_DDHRESULT(2085)
#define D3DERR_INVALIDSTATEBLOCK	    MAKE_DDHRESULT(2100)
#define D3DERR_INBEGINSTATEBLOCK	    MAKE_DDHRESULT(2101)
#define D3DERR_NOTINBEGINSTATEBLOCK	    MAKE_DDHRESULT(2102)

static ErrorCodeItem ec_d3d_items[] = {
    REGCODE(D3D_OK)
    REGCODE(D3DERR_BADMAJORVERSION)
    REGCODE(D3DERR_BADMINORVERSION)
    REGCODE(D3DERR_INVALID_DEVICE)
    REGCODE(D3DERR_INITFAILED)
    REGCODE(D3DERR_DEVICEAGGREGATED)
    REGCODE(D3DERR_EXECUTE_CREATE_FAILED)
    REGCODE(D3DERR_EXECUTE_DESTROY_FAILED)
    REGCODE(D3DERR_EXECUTE_LOCK_FAILED)
    REGCODE(D3DERR_EXECUTE_UNLOCK_FAILED)
    REGCODE(D3DERR_EXECUTE_LOCKED)
    REGCODE(D3DERR_EXECUTE_NOT_LOCKED)
    REGCODE(D3DERR_EXECUTE_FAILED)
    REGCODE(D3DERR_EXECUTE_CLIPPED_FAILED)
    REGCODE(D3DERR_TEXTURE_NO_SUPPORT)
    REGCODE(D3DERR_TEXTURE_CREATE_FAILED)
    REGCODE(D3DERR_TEXTURE_DESTROY_FAILED)
    REGCODE(D3DERR_TEXTURE_LOCK_FAILED)
    REGCODE(D3DERR_TEXTURE_UNLOCK_FAILED)
    REGCODE(D3DERR_TEXTURE_LOAD_FAILED)
    REGCODE(D3DERR_TEXTURE_SWAP_FAILED)
    REGCODE(D3DERR_TEXTURE_LOCKED)
    REGCODE(D3DERR_TEXTURE_NOT_LOCKED)
    REGCODE(D3DERR_TEXTURE_GETSURF_FAILED)
    REGCODE(D3DERR_MATRIX_CREATE_FAILED)
    REGCODE(D3DERR_MATRIX_DESTROY_FAILED)
    REGCODE(D3DERR_MATRIX_SETDATA_FAILED)
    REGCODE(D3DERR_MATRIX_GETDATA_FAILED)
    REGCODE(D3DERR_SETVIEWPORTDATA_FAILED)
    REGCODE(D3DERR_INVALIDCURRENTVIEWPORT)
    REGCODE(D3DERR_INVALIDPRIMITIVETYPE)
    REGCODE(D3DERR_INVALIDVERTEXTYPE)
    REGCODE(D3DERR_TEXTURE_BADSIZE)
    REGCODE(D3DERR_INVALIDRAMPTEXTURE)
    REGCODE(D3DERR_MATERIAL_CREATE_FAILED)
    REGCODE(D3DERR_MATERIAL_DESTROY_FAILED)
    REGCODE(D3DERR_MATERIAL_SETDATA_FAILED)
    REGCODE(D3DERR_MATERIAL_GETDATA_FAILED)
    REGCODE(D3DERR_INVALIDPALETTE)
    REGCODE(D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY)
    REGCODE(D3DERR_ZBUFF_NEEDS_VIDEOMEMORY)
    REGCODE(D3DERR_SURFACENOTINVIDMEM)
    REGCODE(D3DERR_LIGHT_SET_FAILED)
    REGCODE(D3DERR_LIGHTHASVIEWPORT)
    REGCODE(D3DERR_LIGHTNOTINTHISVIEWPORT)
    REGCODE(D3DERR_SCENE_IN_SCENE)
    REGCODE(D3DERR_SCENE_NOT_IN_SCENE)
    REGCODE(D3DERR_SCENE_BEGIN_FAILED)
    REGCODE(D3DERR_SCENE_END_FAILED)
    REGCODE(D3DERR_INBEGIN)
    REGCODE(D3DERR_NOTINBEGIN)
    REGCODE(D3DERR_NOVIEWPORTS)
    REGCODE(D3DERR_VIEWPORTDATANOTSET)
    REGCODE(D3DERR_VIEWPORTHASNODEVICE)
    REGCODE(D3DERR_NOCURRENTVIEWPORT)
    REGCODE(D3DERR_INVALIDVERTEXFORMAT)
    REGCODE(D3DERR_COLORKEYATTACHED)
    REGCODE(D3DERR_VERTEXBUFFEROPTIMIZED)
    REGCODE(D3DERR_VBUF_CREATE_FAILED)
    REGCODE(D3DERR_VERTEXBUFFERLOCKED)
    REGCODE(D3DERR_VERTEXBUFFERUNLOCKFAILED)
    REGCODE(D3DERR_ZBUFFER_NOTPRESENT)
    REGCODE(D3DERR_STENCILBUFFER_NOTPRESENT)
    REGCODE(D3DERR_WRONGTEXTUREFORMAT)
    REGCODE(D3DERR_UNSUPPORTEDCOLOROPERATION)
    REGCODE(D3DERR_UNSUPPORTEDCOLORARG)
    REGCODE(D3DERR_UNSUPPORTEDALPHAOPERATION)
    REGCODE(D3DERR_UNSUPPORTEDALPHAARG)
    REGCODE(D3DERR_TOOMANYOPERATIONS)
    REGCODE(D3DERR_CONFLICTINGTEXTUREFILTER)
    REGCODE(D3DERR_UNSUPPORTEDFACTORVALUE)
    REGCODE(D3DERR_CONFLICTINGRENDERSTATE)
    REGCODE(D3DERR_UNSUPPORTEDTEXTUREFILTER)
    REGCODE(D3DERR_TOOMANYPRIMITIVES)
    REGCODE(D3DERR_INVALIDMATRIX)
    REGCODE(D3DERR_TOOMANYVERTICES)
    REGCODE(D3DERR_CONFLICTINGTEXTUREPALETTE)
    REGCODE(D3DERR_DRIVERINTERNALERROR)
    REGCODE(D3DERR_INVALIDSTATEBLOCK)
    REGCODE(D3DERR_INBEGINSTATEBLOCK)
    REGCODE(D3DERR_NOTINBEGINSTATEBLOCK)
    REGCODE(D3DERR_NOTFOUND)
    REGCODE(D3DERR_MOREDATA)
    REGCODE(D3DERR_DEVICELOST)
    REGCODE(D3DERR_DEVICENOTRESET)
    REGCODE(D3DERR_NOTAVAILABLE)
    REGCODE(D3DERR_OUTOFVIDEOMEMORY)
    REGCODE(D3DERR_INVALIDDEVICE)
    REGCODE(D3DERR_INVALIDCALL)
    REGCODE(D3DERR_DRIVERINVALIDCALL)
    REGCODE(D3DERR_WASSTILLDRAWING)
    REGCODE(D3DOK_NOAUTOGEN)
    REGCODE(D3DERR_DEVICEREMOVED)
    REGCODE(S_NOT_RESIDENT)
    REGCODE(S_RESIDENT_IN_SHARED_MEMORY)
    REGCODE(S_PRESENT_MODE_CHANGED)
    REGCODE(S_PRESENT_OCCLUDED)
    REGCODE(D3DERR_DEVICEHUNG)
    REGCODE(D3DERR_UNSUPPORTEDOVERLAY)
    REGCODE(D3DERR_UNSUPPORTEDOVERLAYFORMAT)
    REGCODE(D3DERR_CANNOTPROTECTCONTENT)
    REGCODE(D3DERR_UNSUPPORTEDCRYPTO)
    REGCODE(D3DERR_PRESENT_STATISTICS_DISJOINT)
};

static LPTSTR d3d_code2msg(DWORD dwCode)
{
    switch (dwCode)
    {
        case D3D_OK:
        return TEXT("No error occurred.");

        case D3DERR_BADMAJORVERSION:
        return TEXT("The service that you requested is unavailable in this major version of DirectX.");

        case D3DERR_BADMINORVERSION:
        return TEXT("The service that you requested is available in this major version of DirectX, but not in this minor version.");

        case D3DERR_INVALID_DEVICE:
        return TEXT("The requested device type is not valid.");

        case D3DERR_INITFAILED:
        return TEXT("A rendering device could not be created because the new device could not be initialized.");

        case D3DERR_DEVICEAGGREGATED:
        return TEXT("The ID3DDevice::SetRenderTarget() method was called on a device that was retrieved from the render target surface.");

        case D3DERR_EXECUTE_CREATE_FAILED:
        return TEXT("The execute buffer could not be created. This typically occurs when no memory is available to allocate the execute buffer.");

        case D3DERR_EXECUTE_DESTROY_FAILED:
        return TEXT("The memory for the execute buffer could not be deallocated.");

        case D3DERR_EXECUTE_LOCK_FAILED:
        return TEXT("The execute buffer could not be locked.");

        case D3DERR_EXECUTE_UNLOCK_FAILED:
        return TEXT("The execute buffer could not be unlocked.");

        case D3DERR_EXECUTE_LOCKED:
        return TEXT("The operation requested by the application could not be completed because the execute buffer is locked.");

        case D3DERR_EXECUTE_NOT_LOCKED:
        return TEXT("The execute buffer could not be unlocked because it is not currently locked.");

        case D3DERR_EXECUTE_FAILED:
        return TEXT("The contents of the execute buffer are invalid and cannot be executed.");

        case D3DERR_EXECUTE_CLIPPED_FAILED:
        return TEXT("The execute buffer could not be clipped during execution.");

        case D3DERR_TEXTURE_NO_SUPPORT:
        return TEXT("The device does not support texture mapping.");

        case D3DERR_TEXTURE_CREATE_FAILED:
        return TEXT("The texture handle for the texture could not be retrieved from the driver.");

        case D3DERR_TEXTURE_DESTROY_FAILED:
        return TEXT("The device was unable to deallocate the texture memory.");

        case D3DERR_TEXTURE_LOCK_FAILED:
        return TEXT("The texture could not be locked.");

        case D3DERR_TEXTURE_UNLOCK_FAILED:
        return TEXT("The texture surface could not be unlocked.");

        case D3DERR_TEXTURE_LOAD_FAILED:
        return TEXT("The texture could not be loaded.");

        case D3DERR_TEXTURE_SWAP_FAILED:
        return TEXT("The texture handles could not be swapped.");

        case D3DERR_TEXTURE_LOCKED:
        return TEXT("The requested operation could not be completed because the texture surface is currently locked.");

        case D3DERR_TEXTURE_NOT_LOCKED:
        return TEXT("The requested operation could not be completed because the texture surface is not locked.");

        case D3DERR_TEXTURE_GETSURF_FAILED:
        return TEXT("The DirectDraw surface used to create the texture could not be retrieved.");

        case D3DERR_MATRIX_CREATE_FAILED:
        return TEXT("The matrix could not be created. This can occur when no memory is available to allocate for the matrix.");

        case D3DERR_MATRIX_DESTROY_FAILED:
        return TEXT("The memory for the matrix could not be deallocated.");

        case D3DERR_MATRIX_SETDATA_FAILED:
        return TEXT("The matrix data could not be set. This can occur when the matrix was not created by the current device.");

        case D3DERR_MATRIX_GETDATA_FAILED:
        return TEXT("The matrix data could not be retrieved. This can occur when the matrix was not created by the current device.");

        case D3DERR_SETVIEWPORTDATA_FAILED:
        return TEXT("The viewport parameters could not be set.");

        case D3DERR_INVALIDCURRENTVIEWPORT:
        return TEXT("The currently selected viewport is not valid.");

        case D3DERR_INVALIDPRIMITIVETYPE:
        return TEXT("The primitive type specified by the application is invalid.");

        case D3DERR_INVALIDVERTEXTYPE:
        return TEXT("The vertex type specified by the application is invalid.");

        case D3DERR_TEXTURE_BADSIZE:
        return TEXT("The dimensions of a current texture are invalid. This can occur when an application attempts to use a texture that has dimensions that are not a power of 2 with a device that requires them.");

        case D3DERR_INVALIDRAMPTEXTURE:
        return TEXT("Ramp mode is being used, and the texture handle in the current material does not match the current texture handle that is set as a render state.");

        case D3DERR_MATERIAL_CREATE_FAILED:
        return TEXT("The material could not be created. This typically occurs when no memory is available to allocate for the material.");

        case D3DERR_MATERIAL_DESTROY_FAILED:
        return TEXT("The memory for the material could not be deallocated.");

        case D3DERR_MATERIAL_SETDATA_FAILED:
        return TEXT("The material parameters could not be set.");

        case D3DERR_MATERIAL_GETDATA_FAILED:
        return TEXT("The material parameters could not be retrieved.");

        case D3DERR_INVALIDPALETTE:
        return TEXT("The palette associated with a surface is invalid.");

        case D3DERR_ZBUFF_NEEDS_SYSTEMMEMORY:
        return TEXT("The requested operation could not be completed because the specified device requires system-memory depth-buffer surfaces.");

        case D3DERR_ZBUFF_NEEDS_VIDEOMEMORY:
        return TEXT("The requested operation could not be completed because the specified device requires video-memory depth-buffer surfaces.");

        case D3DERR_SURFACENOTINVIDMEM:
        return TEXT("The device could not be created because the render target surface is not located in video memory.");

        case D3DERR_LIGHT_SET_FAILED:
        return TEXT("The attempt to set lighting parameters for a light object failed.");

        case D3DERR_LIGHTHASVIEWPORT:
        return TEXT("The requested operation failed because the light object is associated with another viewport.");

        case D3DERR_LIGHTNOTINTHISVIEWPORT:
        return TEXT("The requested operation failed because the light object has not been associated with this viewport.");

        case D3DERR_SCENE_IN_SCENE:
        return TEXT("Scene rendering could not begin because a previous scene was not completed by a call to the ID3DDevice::EndScene() method.");

        case D3DERR_SCENE_NOT_IN_SCENE:
        return TEXT("Scene rendering could not be completed because a scene was not started by a previous call to the ID3DDevice::BeginScene() method.");

        case D3DERR_SCENE_BEGIN_FAILED:
        return TEXT("Scene rendering could not begin.");

        case D3DERR_SCENE_END_FAILED:
        return TEXT("Scene rendering could not be completed.");

        case D3DERR_INBEGIN:
        return TEXT("The requested operation cannot be completed while scene rendering is taking place.");

        case D3DERR_NOTINBEGIN:
        return TEXT("The requested rendering operation could not be completed because scene rendering has not begun.");

        case D3DERR_NOVIEWPORTS:
        return TEXT("The requested operation failed because the device currently has no viewports associated with it.");

        case D3DERR_VIEWPORTDATANOTSET:
        return TEXT("The requested operation could not be completed because viewport parameters have not yet been set.");

        case D3DERR_VIEWPORTHASNODEVICE:
        return TEXT("The requested operation could not be completed because the viewport has not yet been associated with a device.");

        case D3DERR_NOCURRENTVIEWPORT:
        return TEXT("The viewport parameters could not be retrieved because none have been set.");

        case D3DERR_INVALIDVERTEXFORMAT:
        return TEXT("The combination of flexible vertex format flags specified by the application is not valid.");

        case D3DERR_COLORKEYATTACHED:
        return TEXT("The application attempted to create a texture with a surface that uses a color key for transparency.");

        case D3DERR_VERTEXBUFFEROPTIMIZED:
        return TEXT("The requested operation could not be completed because the vertex buffer is optimized.");

        case D3DERR_VBUF_CREATE_FAILED:
        return TEXT("The vertex buffer could not be created. This can happen when there is insufficient memory to allocate a vertex buffer.");

        case D3DERR_VERTEXBUFFERLOCKED:
        return TEXT("The requested operation could not be completed because the vertex buffer is locked.");

        case D3DERR_VERTEXBUFFERUNLOCKFAILED:
        return TEXT("The vertex buffer could not be unlocked because the vertex buffer memory was overrun. Be sure that your application does not write beyond the size of the vertex buffer.");

        case D3DERR_ZBUFFER_NOTPRESENT:
        return TEXT("The requested operation could not be completed because the render target surface does not have an attached depth buffer.");

        case D3DERR_STENCILBUFFER_NOTPRESENT:
        return TEXT("The DirectDraw surface used to create the texture could not be retrieved.");

        case D3DERR_WRONGTEXTUREFORMAT:
        return TEXT("The pixel format of the texture surface is not valid.");

        case D3DERR_UNSUPPORTEDCOLOROPERATION:
        return TEXT("The device does not support a specified texture-blending operation for color values.");

        case D3DERR_UNSUPPORTEDCOLORARG:
        return TEXT("The device does not support a specified texture-blending argument for color values.");

        case D3DERR_UNSUPPORTEDALPHAOPERATION:
        return TEXT("The device does not support a specified texture-blending operation for the alpha channel.");

        case D3DERR_UNSUPPORTEDALPHAARG:
        return TEXT("The device does not support a specified texture-blending argument for the alpha channel.");

        case D3DERR_TOOMANYOPERATIONS:
        return TEXT("The application is requesting more texture-filtering operations than the device supports.");

        case D3DERR_CONFLICTINGTEXTUREFILTER:
        return TEXT("The current texture filters cannot be used together.");

        case D3DERR_UNSUPPORTEDFACTORVALUE:
        return TEXT("The device does not support the specified texture factor value. Not used; provided only to support older drivers.");

        case D3DERR_CONFLICTINGRENDERSTATE:
        return TEXT("The currently set render states cannot be used together.");

        case D3DERR_UNSUPPORTEDTEXTUREFILTER:
        return TEXT("The device does not support the specified texture filter.");

        case D3DERR_TOOMANYPRIMITIVES:
        return TEXT("The device is unable to render the provided number of primitives in a single pass.");

        case D3DERR_INVALIDMATRIX:
        return TEXT("The requested operation could not be completed because the combination of the currently set world, view, and projection matrices is invalid (the determinant of the combined matrix is 0).");

        case D3DERR_TOOMANYVERTICES:
        return TEXT("The device is unable to process the provided number of vertices in a single pass.");

        case D3DERR_CONFLICTINGTEXTUREPALETTE:
        return TEXT("The current textures cannot be used simultaneously.");

        case D3DERR_DRIVERINTERNALERROR:
        return TEXT("Internal driver error. Applications should destroy and recreate the device when receiving this error.");

        case D3DERR_INVALIDSTATEBLOCK:
        return TEXT("The state block handle is invalid.");

        case D3DERR_INBEGINSTATEBLOCK:
        return TEXT("The operation cannot be completed while recording states for a state block. Complete recording by calling the ID3DDevice::EndStateBlock() method, and try again.");

        case D3DERR_NOTINBEGINSTATEBLOCK:
        return TEXT("The requested operation could not be completed because it is only valid while recording a state block. Call the ID3DDevice::BeginStateBlock() method, and try again.");

        case D3DERR_NOTFOUND:
        return TEXT("The requested item was not found.");

        case D3DERR_MOREDATA:
        return TEXT("There is more data available than the specified buffer size can hold.");

        case D3DERR_DEVICELOST:
        return TEXT("The device has been lost but cannot be reset at this time. Therefore, rendering is not possible. A Direct3D device object other than the one that returned this code caused the hardware adapter to be reset by the OS. Delete all video memory objects (surfaces, textures, state blocks) and call Reset() to return the device to a default state. If the application continues rendering without a reset, the rendering calls will succeed.");

        case D3DERR_DEVICENOTRESET:
        return TEXT("The device has been lost but can be reset at this time.");

        case D3DERR_NOTAVAILABLE:
        return TEXT("This device does not support the queried technique.");

        case D3DERR_OUTOFVIDEOMEMORY:
        return TEXT("Direct3D does not have enough display memory to perform the operation.");

        case D3DERR_INVALIDDEVICE:
        return TEXT("The requested device type is not valid.");

        case D3DERR_INVALIDCALL:
        return TEXT("The method call is invalid. For example, a method's parameter may not be a valid pointer.");

        case D3DERR_DRIVERINVALIDCALL:
        return TEXT("Not used.");

        case D3DERR_WASSTILLDRAWING:
        return TEXT("The previous blit operation that is transferring information to or from this surface is incomplete.");

        case D3DOK_NOAUTOGEN:
        return TEXT("This is a success code. However, the autogeneration of mipmaps is not supported for this format. This means that resource creation will succeed but the mipmap levels will not be automatically generated.");

        case D3DERR_DEVICEREMOVED:
        return TEXT("The hardware adapter has been removed. Application must destroy the device, do enumeration of adapters and create another Direct3D device. If application continues rendering without calling Reset, the rendering calls will succeed. Applies to Direct3D 9Ex only.");

        case S_NOT_RESIDENT:
        return TEXT("At least one allocation that comprises the resources is on disk. Direct3D 9Ex only.");

        case S_RESIDENT_IN_SHARED_MEMORY:
        return TEXT("No allocations that comprise the resources are on disk. However, at least one allocation is not in GPU-accessible memory. Direct3D 9Ex only.");

        case S_PRESENT_MODE_CHANGED:
        return TEXT("The desktop display mode has been changed. The application can continue rendering, but there might be color conversion/stretching. Pick a back buffer format similar to the current display mode, and call Reset to recreate the swap chains. The device will leave this state after a Reset is called. Direct3D 9Ex only.");

        case S_PRESENT_OCCLUDED:
        return TEXT("The presentation area is occluded. Direct3D 9Ex only.");

        case D3DERR_DEVICEHUNG:
        return TEXT("The device that returned this code caused the hardware adapter to be reset by the OS. Most applications should destroy the device and quit. Applications that must continue should destroy all video memory objects (surfaces, textures, state blocks etc) and call Reset() to put the device in a default state. If the application then continues rendering in the same way, the device will return to this state. Applies to Direct3D 9Ex only.");

        case D3DERR_UNSUPPORTEDOVERLAY:
        return TEXT("The device does not support overlay for the specified size or display mode. Direct3D 9Ex under Windows 7 only.");

        case D3DERR_UNSUPPORTEDOVERLAYFORMAT:
        return TEXT("The device does not support overlay for the specified surface format. Direct3D 9Ex under Windows 7 only.");

        case D3DERR_CANNOTPROTECTCONTENT:
        return TEXT("The specified content cannot be protected. Direct3D 9Ex under Windows 7 only.");

        case D3DERR_UNSUPPORTEDCRYPTO:
        return TEXT("The specified cryptographic algorithm is not supported. Direct3D 9Ex under Windows 7 only.");

        case D3DERR_PRESENT_STATISTICS_DISJOINT:
        return TEXT("The present statistics have no orderly sequence. Direct3D 9Ex under Windows 7 only.");

        default:
        return NULL;
    }
}

static void d3d_freemsg(LPTSTR lpMessage)
{
    /* Ignore */
}

void EcRegD3d(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("Direct3D");
    pecs->pfnMessageFromCode = d3d_code2msg;
    pecs->pfnFreeMessage = d3d_freemsg;
    pecs->items = ec_d3d_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_d3d_items);
}

void EcUnregD3d()
{
    /* Ignore */
}

