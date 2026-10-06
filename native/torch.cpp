#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/binder_parcel.h>

#include <stdio.h>
#include <string.h>

static void* onCreate(void* args) {
    return args;
}

static void onDestroy(void* userData) {
}

static binder_status_t onTransact(AIBinder* binder,
                                  transaction_code_t code,
                                  const AParcel* in,
                                  AParcel* out) {
    return STATUS_UNKNOWN_TRANSACTION;
}

static int set_torch(bool enabled) {
    AIBinder* camera = AServiceManager_getService("media.camera");
    if (!camera) {
        fprintf(stderr, "media.camera service not found\n");
        return 1;
    }

    AIBinder_Class* cls = AIBinder_Class_define(
        "com.alichiddy.torch.Client",
        onCreate,
        onDestroy,
        onTransact);

    if (!cls) {
        fprintf(stderr, "AIBinder_Class_define failed\n");
        AIBinder_decStrong(camera);
        return 1;
    }

    AIBinder* client = AIBinder_new(cls, nullptr);
    if (!client) {
        fprintf(stderr, "AIBinder_new failed\n");
        AIBinder_decStrong(camera);
        return 1;
    }

    AParcel* in = AParcel_create();
    AParcel* out = nullptr;

    if (!in) {
        fprintf(stderr, "AParcel_create failed\n");
        AIBinder_decStrong(client);
        AIBinder_decStrong(camera);
        return 1;
    }

    binder_status_t s;
    s = AParcel_writeString(
        in, "android.hardware.ICameraService", 32);
    if (s != STATUS_OK) goto fail;

    s = AParcel_writeString(in, "0", 1);
    if (s != STATUS_OK) goto fail;

    s = AParcel_writeInt32(in, enabled ? 1 : 0);
    if (s != STATUS_OK) goto fail;

    s = AParcel_writeStrongBinder(in, client);
    if (s != STATUS_OK) goto fail;

    /*
     * Vivo's Android 12 camera service exposes SET_TORCH_MODE
     * at transaction 16 on this device.
     */
    s = AIBinder_transact(camera, 16, &in, &out, 0);
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

    AParcel_delete(out);
    AIBinder_decStrong(client);
    AIBinder_decStrong(camera);

    return result == 0 ? 0 : 1;

fail:
    AParcel_delete(in);
    AParcel_delete(out);
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
