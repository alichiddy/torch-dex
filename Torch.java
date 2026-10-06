import android.content.Context;
import android.hardware.camera2.CameraManager;
import android.os.Looper;
import java.io.File;

public class Torch {
    public static void main(String[] a) throws Exception {
        if (a.length != 1 ||
            !(a[0].equals("on") ||
              a[0].equals("off") ||
              a[0].equals("toggle"))) {
            System.err.println("usage: Torch on|off|toggle");
            System.exit(2);
        }

        Looper.prepareMainLooper();

        Class<?> at = Class.forName("android.app.ActivityThread");
        Object t = at.getMethod("systemMain").invoke(null);
        Context c = (Context) at.getMethod("getSystemContext").invoke(t);

        CameraManager cm =
            (CameraManager) c.getSystemService(Context.CAMERA_SERVICE);

        File f = new File("/data/local/tmp/torch.state");

        boolean on =
            a[0].equals("on") ||
            (a[0].equals("toggle") && !f.exists());

        cm.setTorchMode("0", on);

        if (on) {
            f.createNewFile();
        } else {
            f.delete();
        }

        System.exit(0);
    }
}
