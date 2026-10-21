// SPDX-License-Identifier: MIT
// Copyright (c) 2026 WarcraftXL

#include "wxl/sdk.hpp"
#include "wxl/services/script.hpp"

namespace
{
    constexpr char kTag[] = "torghast-modelscene";

    // The stock 3.3.5 client has Model frames but no Retail ModelScene/AddEffect API. This shim
    // creates real 3D Model widgets and effect controllers. Model paths are registered by the
    // companion addon because Retail M2s cannot be loaded directly by the WotLK model reader.
    constexpr char kInstallShim[] = R"lua(
if not WXLModelSceneCompat then
    local compat = { effects = {}, scenes = {} }
    WXLModelSceneCompat = compat

    local Controller = {}
    Controller.__index = Controller

    function Controller:Stop()
        if self.stopped then return end
        self.stopped = true
        if self.frame then
            self.frame:Hide()
            self.frame:SetScript("OnUpdate", nil)
            self.frame:ClearModel()
            self.frame:SetParent(nil)
            self.frame = nil
        end
        local scene = self.scene
        if scene and scene.controllers then scene.controllers[self] = nil end
    end

    function Controller:SetPaused(paused)
        self.paused = not not paused
        if self.frame then
            if self.paused then self.frame:Hide() else self.frame:Show() end
        end
    end

    function Controller:IsPlaying()
        return not self.stopped and self.frame ~= nil
    end

    function Controller:SetModelScale(scale)
        self.scale = scale
        if self.frame then self.frame:SetModelScale(scale) end
    end

    function Controller:SetAlpha(alpha)
        self.alpha = alpha
        if self.frame then self.frame:SetAlpha(alpha) end
    end

    local Scene = {}
    Scene.__index = Scene

    function Scene:AddEffect(effectID, anchor, options)
        local definition = compat.effects[effectID]
        if not definition then
            geterrorhandler()(format("WXL ModelScene: effect %s has no WotLK model mapping", tostring(effectID)))
            return nil
        end
        options = options or {}
        local parent = anchor or self.owner or UIParent
        local frame = CreateFrame("Model", nil, parent)
        frame:ClearAllPoints()
        if options.width or options.height then
            frame:SetSize(options.width or parent:GetWidth(), options.height or parent:GetHeight())
            frame:SetPoint("CENTER", parent, "CENTER", options.offsetX or 0, options.offsetY or 0)
        else
            frame:SetAllPoints(parent)
        end
        frame:SetFrameStrata(options.frameStrata or "MEDIUM")
        frame:SetFrameLevel((parent.GetFrameLevel and parent:GetFrameLevel() or 1) + (options.frameLevelOffset or 1))
        frame:SetModel(definition.model)
        if definition.sequence and frame.SetSequence then frame:SetSequence(definition.sequence) end
        if frame.SetModelScale then frame:SetModelScale(options.scale or definition.scale or 1) end
        frame:SetAlpha(options.alpha or definition.alpha or 1)
        frame:Show()

        local controller = setmetatable({ scene = self, frame = frame, effectID = effectID }, Controller)
        self.controllers[controller] = true
        local duration = options.duration or definition.duration
        if duration and duration > 0 then
            local elapsed = 0
            frame:SetScript("OnUpdate", function(_, delta)
                elapsed = elapsed + delta
                if elapsed >= duration then controller:Stop() end
            end)
        end
        return controller
    end

    function Scene:Clear()
        for controller in pairs(self.controllers) do controller:Stop() end
    end

    function Scene:SetOwner(owner)
        self.owner = owner
    end

    function compat.RegisterEffect(effectID, modelPath, options)
        assert(type(effectID) == "number", "effectID must be a number")
        assert(type(modelPath) == "string" and modelPath ~= "", "modelPath must be a WotLK-compatible .m2 path")
        options = options or {}
        compat.effects[effectID] = {
            model = modelPath,
            sequence = options.sequence,
            scale = options.scale,
            alpha = options.alpha,
            duration = options.duration,
        }
    end

    function compat.CreateScene(owner)
        return setmetatable({ owner = owner, controllers = {} }, Scene)
    end

    local function installGlobal(name)
        if _G[name] then return _G[name] end
        local scene = compat.CreateScene(UIParent)
        _G[name] = scene
        return scene
    end

    installGlobal("GlobalFXBackgroundModelScene")
    installGlobal("GlobalFXMediumModelScene")
    installGlobal("GlobalFXHighModelScene")
end
)lua";

    class TorghastModelScene final : public wxl::Mod, public wxl::LuaScript
    {
    public:
        wxl_result OnLoad() override
        {
            wxl::ScriptMgr::Add(this);
            return WXL_OK;
        }

        void OnScriptVmReady(const wxl_script_vm_args& args) override
        {
            if (args.context != WXL_SCRIPT_CONTEXT_GAME)
                return;

            const wxl::ScriptService script;
            if (!script)
            {
                wxl::Log(WXL_LOG_WARN, kTag, "wxl.script service is unavailable");
                return;
            }

            const wxl_result result = script.Run(kInstallShim, "WXL Torghast ModelScene compatibility");
            if (result != WXL_OK)
                wxl::Log(WXL_LOG_ERROR, kTag, "failed to install ModelScene compatibility shim");
            else
                wxl::Log(WXL_LOG_INFO, kTag, "installed 3D ModelScene compatibility shim");
        }
    };
} // namespace

WXL_MOD(TorghastModelScene, "wxl.torghast-modelscene", "Torghast ModelScene compatibility", 1)
