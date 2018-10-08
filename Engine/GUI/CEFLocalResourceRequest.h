// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#include "Define.h"
// ------------------------------------ //
#include "include/cef_scheme.h"

namespace Leviathan { namespace GUI {

//! \brief Provides support for loading local resources from "leviathan-local" with the http
//! protocol
class CefLocalResourceRequestHandlerFactory : public CefSchemeHandlerFactory {
public:
    CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, const CefString& scheme_name,
        CefRefPtr<CefRequest> request) override;

    IMPLEMENT_REFCOUNTING(CefLocalResourceRequestHandlerFactory);
};

}} // namespace Leviathan::GUI
