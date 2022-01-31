import java.nio.ByteBuffer;

class LibRetro {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
    public static void render(ByteBuffer buf){
        System.out.println("got "+buf);
        buf.putInt(200, 0xffff);



    }
}