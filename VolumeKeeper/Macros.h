#pragma once

#define EXIT_ON_ERROR(hr)  \
              if (FAILED(hr)) { goto Exit; }

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

#define CHECK_HR(x) if (FAILED(x)) { goto Exit; }
