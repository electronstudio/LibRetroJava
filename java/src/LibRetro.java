import java.awt.*;
import java.awt.color.ColorSpace;
import java.awt.image.*;
import java.nio.ByteBuffer;
import java.util.Random;


class LibRetro {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }

    static int i=1;

    static Random rand = new Random();

    public static void render(ByteBuffer buf) {
        //System.out.println("got " + buf);
        buf.putInt(200, 0xffff);

        Dimension dim = new Dimension(320, 240);
        int size = dim.width * dim.height * 4;


        DataBuffer dbuf = new DataBuffer(DataBuffer.TYPE_BYTE, size) {
            @Override
            public void setElem(int bank, int i, int val) {
                buf.put(i, (byte) val);
            }

            @Override
            public int getElem(int bank, int i) {
                return buf.get(i);
            }
        };

        ColorSpace cs = ColorSpace.getInstance(ColorSpace.CS_sRGB);
        ColorModel cm = new ComponentColorModel(cs, new int[]{8, 8, 8, 8}, true, false, Transparency.TRANSLUCENT, DataBuffer.TYPE_BYTE);

        SampleModel sm = cm.createCompatibleSampleModel(dim.width, dim.height);
        WritableRaster raster = new WritableRaster(sm, dbuf, new Point()) {
        };
        BufferedImage img = new BufferedImage(cm, raster, false, null);


        Graphics g = img.getGraphics();
        g.setColor(Color.BLACK);
        g.fillRect(0, 0, 320, 240);
        g.setColor(Color.RED);
        g.fillRect(i, 0, 20, 20);
        //g.drawString("Point is here", i++, 20);
        g.fillRect(0, i++, 20, 20);
        if(i >=320) i=0;

        g.dispose();  // get rid of the Graphics context to save resources

       //for(int i=0; i<200; i++) {
       // i = rand.nextInt(200);
//
//        int[] c = new int[]{0xff, 0x00, 0xff, 0x00};
//            raster.setPixel(30, i, c);
//        raster.setPixel(31, i, c);
//        raster.setPixel(30, i+1, c);
//        raster.setPixel(31, i+1, c);
//        System.out.println("got " + i);
       //}

    }
}