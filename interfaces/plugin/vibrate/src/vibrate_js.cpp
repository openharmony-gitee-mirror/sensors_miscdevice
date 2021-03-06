/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <map>
#include <string>
#include <unistd.h>

#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "vibrator_agent.h"
#include "vibrator_napi_utils.h"

using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = { LOG_CORE, 0xD002757, "VibrateJsAPI" };
constexpr int32_t ARGS_LENGTH = 2;

static napi_value Vibrate(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    napi_value thisArg;
    napi_value result;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr));
    if (argc == ARGS_LENGTH) {
        if (!IsMatchType(args[1], napi_function, env) ||
            (!IsMatchType(args[0], napi_number, env) && !IsMatchType(args[0], napi_string, env))) {
            HiLog::Error(LABEL, "%{public}s input parameter type does not match", __func__);
            napi_get_undefined(env, &result);
            return result;
        }
        AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
        napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]);
        if (IsMatchType(args[0], napi_number, env)) {
            int32_t duration = GetCppInt32(args[0], env);
            asyncCallbackInfo->status = StartVibratorOnce(duration);
        } else if (IsMatchType(args[0], napi_string, env)) {
            size_t bufLength = 0;
            napi_status status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &bufLength);
            char *buff = (char *)malloc((bufLength + 1) * sizeof(char));
            status = napi_get_value_string_utf8(env, args[0], buff, bufLength + 1, &bufLength);
            asyncCallbackInfo->status = StartVibrator(buff);
        }
        EmitAsyncCallbackWork(asyncCallbackInfo);
    } else if (argc == 1) {
        AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;
        if (IsMatchType(args[0], napi_number, env)) {
            int32_t duration = GetCppInt32(args[0], env);
            asyncCallbackInfo->status = StartVibratorOnce(duration);
        } else if (IsMatchType(args[0], napi_string, env)) {
            size_t bufLength = 0;
            napi_status status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &bufLength);
            char *buff = (char *)malloc((bufLength + 1) * sizeof(char));
            status = napi_get_value_string_utf8(env, args[0], buff, bufLength + 1, &bufLength);
            asyncCallbackInfo->status = StartVibrator(buff);
        } else {
            HiLog::Error(LABEL, "%{public}s input parameter type does not match", __func__);
            return nullptr;
        }
        EmitPromiseWork(asyncCallbackInfo);
        return promise;
    } else {
        HiLog::Error(LABEL, "%{public}s the number of input parameters does not match", __func__);
    }
    napi_get_undefined(env, &result);
    return result;
}

static napi_value Stop(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2];
    napi_value thisArg;
    napi_value result;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr));
    if (argc == ARGS_LENGTH) {
        if (!IsMatchType(args[1], napi_function, env) || !IsMatchType(args[0], napi_string, env)) {
            HiLog::Error(LABEL, "%{public}s input parameter type does not match", __func__);
            napi_get_undefined(env, &result);
            return result;
        }
        AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
        napi_create_reference(env, args[1], 1, &asyncCallbackInfo->callback[0]);
        const char *mode = GetCppString(args[0], env).c_str();
        asyncCallbackInfo->status = StopVibrator(mode);
        EmitAsyncCallbackWork(asyncCallbackInfo);
    } else if (argc == 1) {
        if (!IsMatchType(args[0], napi_string, env)) {
            HiLog::Error(LABEL, "%{public}s input parameter type does not match", __func__);
            return nullptr;
        }
        AsyncCallbackInfo *asyncCallbackInfo = new AsyncCallbackInfo {
            .env = env,
            .asyncWork = nullptr,
            .deferred = nullptr,
        };
        napi_deferred deferred;
        napi_value promise;
        NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
        asyncCallbackInfo->deferred = deferred;
        const char *mode = GetCppString(args[0], env).c_str();
        asyncCallbackInfo->status = StopVibrator(mode);
        EmitPromiseWork(asyncCallbackInfo);
        return promise;
    } else {
        HiLog::Error(LABEL, "%{public}s the number of input parameters does not match", __func__);
    }
    napi_get_undefined(env, &result);
    return result;
}

EXTERN_C_START

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("vibrate", Vibrate),
        DECLARE_NAPI_FUNCTION("stop", Stop)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "vibrator",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
