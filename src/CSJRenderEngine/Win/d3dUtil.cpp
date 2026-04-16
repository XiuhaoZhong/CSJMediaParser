#include "d3dUtil.h"

HRESULT CreateShaderFromFile(const WCHAR * csoFileNameOut, 
                             const WCHAR * hisFileName, 
                             LPCSTR entryPoint, 
                             LPCSTR shaderModel, 
                             ID3DBlob ** ppBlocbOut) {
    
    HRESULT hr = S_OK;

    /* Find the vertex shader which already be compiled */ 
    if (csoFileNameOut && D3DReadFileToBlob(csoFileNameOut, ppBlocbOut) == S_OK) {
        return hr;
    } else {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        /* Setting D3DCOMPILE_DEBUG flag is to get the shader's debug infomation.
         * Setting this flag could improve the debug experience.
         *
         **/
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        /* Disable the optimizing under the DEBUG enviroment to avoid some 
           unreasonable case.
        */
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ID3DBlob* errorBlob = nullptr;
        hr = D3DCompileFromFile(hisFileName,
                                nullptr,
                                D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                entryPoint,
                                shaderModel,
                                dwShaderFlags,
                                0,
                                ppBlocbOut,
                                &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob != nullptr) {
                OutputDebugString(reinterpret_cast<const WCHAR*>(errorBlob->GetBufferPointer()));
            }

            SAFE_RELEASE(errorBlob);
            return hr;
        }

        if (csoFileNameOut) {
            return D3DWriteBlobToFile(*ppBlocbOut, csoFileNameOut, FALSE);
        }
    }
    
    return hr;
}
