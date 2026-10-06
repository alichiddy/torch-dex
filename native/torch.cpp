#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/binder_parcel.h>
#include <android/binder_process.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static binder_status_t onTransact(AIBinder* binder, transaction_code_t code,
                                  const AParcel* in, AParcel* out) {
    return STATUS_UNKNOWN_TRANSACTION;
}

static AIBinder_Class* makeClass() {
    AIBinder_Class* cls = AIBinder_Class_define(
        "com.alichiddy.torch.Client",
        nullptr,
        onTransact,
        nullptr);
    return cls;
}

static int set_torch(bool enabled) {
    AIBinder* camera = AServiceManager_getService("media.camera");
    if (!camera) {
        fprintf(stderr, "media.camera service not found\n");
        return 1;
    }

    AIBinder_Class* cls = makeClass();
    if (!cls) {
        fprintf(stderr, "AIBinder_Class_define failed\n");
        return 1;
    }

    AIBinder* client = AIBinder_new(cls, nullptr);
    if (!client) {
        fprintf(stderr, "AIBinder_new failed\n");
        return 1;
    }

    AParcel* in = AParcel_create();
    AParcel* out = AParcel_create();
    if (!in || !out) {
        fprintf(stderr, "AParcel_create failed\n");
        return 1;
    }

    binder_status_t s;
    s = AParcel_writeInterfaceToken(in, "android.hardware.ICameraService");
    if (s != STATUS_OK) goto fail;
    s = AParcel_writeString(in, "0");
    if (s != STATUS_OK) goto fail;
    s = AParcel_writeInt32(in, enabled ? 1 : 0);
    if (s != STATUS_OK) goto fail;
    s = AParcel_writeStrongBinder(in, client);
    if (s != STATUS_OK) goto fail;

    s = AIBinder_transact(camera, 12, in, out, 0);
    if (s != STATUS_OK) {
        fprintf(stderr, "binder transact failed: %d\n", (int)s);
        goto fail;
    }

    int32_t exception = 0;
    s = AParcel_readInt32(out, &exception);
    if (s != STATUS_OK) goto fail;
    if (exception != 0) {
        fprintf(stderr, "camera service exception: %d\n", exception);
        goto fail;
    }

    int32_t result = -999;
    s = AParcel_readInt32(out, &result);
    if (s != STATUS_OK) goto fail;

    printf("setTorchMode result=%d\n", result);
    if (result != 0) goto fail;

    AParcel_delete(out);
    AParcel_delete(in);
    AIBinder_decStrong(client);
    AIBinder_decStrong(camera);
    return 0;

fail:
    if (out) AParcel_delete(out);
    if (in) AParcel_delete(in);
    AIBinder_decStrong(client);
    AIBinder_decStrong(camera);
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 2 ||
        (strcmp(argv[1], "on") != 0 &&
         strcmp(argv[1], "off") != 0)) {
        fprintf(stderr, "usage: torch_native on|off\n");
        return 2;
    }
    return set_torch(strcmp(argv[1], "on") == 0);
}
