import android.os.Binder;
import android.os.IBinder;
import android.os.Parcel;

import java.io.File;

public class Torch {
    private static final int SET_TORCH_MODE = 16;
    private static final String DESCRIPTOR =
        "android.hardware.ICameraService";

    public static void main(String[] a) throws Exception {
        if (a.length != 1 ||
            !(a[0].equals("on") ||
              a[0].equals("off") ||
              a[0].equals("toggle"))) {
            System.err.println("usage: Torch on|off|toggle");
            System.exit(2);
        }

        File state = new File("/data/local/tmp/torch.state");

        boolean on =
            a[0].equals("on") ||
            (a[0].equals("toggle") && !state.exists());

        Class<?> sm = Class.forName("android.os.ServiceManager");

        java.lang.reflect.Method getService =
            sm.getDeclaredMethod("getService", String.class);

        IBinder camera =
            (IBinder) getService.invoke(null, "media.camera");

        if (camera == null) {
            throw new RuntimeException(
                "media.camera service not found");
        }

        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();

        try {
            data.writeInterfaceToken(DESCRIPTOR);
            data.writeString("0");
            data.writeInt(on ? 1 : 0);

            Binder torchClient = new Binder();
            data.writeStrongBinder(torchClient);

            boolean sent =
                camera.transact(
                    SET_TORCH_MODE,
                    data,
                    reply,
                    0);

            if (!sent) {
                throw new RuntimeException(
                    "Binder transaction failed");
            }

            int result = reply.readInt();

            System.out.println(
                "setTorchMode result=" + result);

            if (result != 0) {
                throw new RuntimeException(
                    "setTorchMode failed: " + result);
            }

            if (on) {
                state.createNewFile();
            } else {
                state.delete();
            }

            System.out.println(
                "TORCH=" + (on ? "ON" : "OFF"));

        } finally {
            reply.recycle();
            data.recycle();
        }
    }
}
