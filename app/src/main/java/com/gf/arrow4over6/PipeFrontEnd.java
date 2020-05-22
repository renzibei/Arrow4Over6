package com.gf.arrow4over6;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public enum PipeFrontEnd {
//    private volatile static PipeFrontEnd _instance;
    INSTANCE;

    private static String pipeDirPath;

    private static final String fromPipeFrontFileName = "FromPipeFront";
    private static final String toPipeFrontFileName = "ToPipeFront";

    private static volatile boolean isReady = false;

    private static final ReentrantReadWriteLock readyLock = new ReentrantReadWriteLock();

    private String fromPipeFrontEndPath;
    private String toPipeFrontEndPath;

    private BufferedInputStream pipeInputStream;
    private BufferedOutputStream pipeOutputStream;

    private void setInstanceFlagReady() {
        readyLock.writeLock().lock();
        isReady = true;
        readyLock.writeLock().unlock();
    }

    PipeFrontEnd() {
        ArLog.i("PipeFrontEnd begins constructing");

    }

    public static boolean isInstanceReady() {
        boolean ret = false;
        readyLock.readLock().lock();
        ret = isReady;
        readyLock.readLock().unlock();
        return ret;
    }

    public static PipeFrontEnd getInstance() {
//        if(_instance == null) {
//            synchronized (PipeFrontEnd.class) {
//                if(_instance == null) {
//                    _instance = new PipeFrontEnd();
//                }
//            }
//        }
//        return _instance;
        return INSTANCE;
    }

    // set pipeDirPath before getInstance
    public static void setPipeDirPathAndBuild(String dirPath) {
        ArLog.i("set pipe dir path to" + dirPath);
        pipeDirPath = dirPath;
        INSTANCE.constructInstance();
    }

    public static void waitUntilInitialized() {
        boolean isPipeReady = false;
        while(!isPipeReady) {
            isPipeReady = PipeFrontEnd.isInstanceReady();
            if(!isPipeReady) {
                try {
                    ArLog.i("Pipe Frontend is not ready");
                    Thread.sleep(100);
                }
                catch (InterruptedException e) {
                    ArLog.e(e.getMessage(), e);
                }
            }
        }
    }

    private void buildInstance() {
        ArLog.i("before build Instance");
        initPipeFilePath();
        makePipe();
        setInstanceFlagReady();
    }

    // Asynchronously construct the instance
    private void constructInstance() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                ArLog.i("asyn construct Instance");
                buildInstance();
            }
        }).start();

    }

    private int readBeginLength() {
        byte[] dataByte = new byte[4];
        try {
            int realLen = pipeInputStream.read(dataByte, 0, 4);
            if(realLen < 4) {
                ArLog.e("read realLen < 4, realLen = " + realLen);
            }
        } catch(IOException e) {
            ArLog.e("Fail to read begin length");
            ArLog.e(e.getMessage(), e);
            return 0;
        }
        ByteBuffer byteBuffer = ByteBuffer.wrap(dataByte);
        byteBuffer.order(ByteOrder.nativeOrder());
        return byteBuffer.getInt();
    }

    public int readInt() {
        int beginLen = readBeginLength();
        if(beginLen != 4) {
            ArLog.e("read begin len for int not equals 4");
            return -1;
        }
        return readBeginLength();
    }

    public byte[] readBytes() {
        int dataLen = readBeginLength();
        byte[] bytes = new byte[dataLen];
        try {
            int realLen = pipeInputStream.read(bytes, 0, dataLen);
            if(realLen < dataLen) {
                ArLog.e("read realLen < targetLen ,  realLen = " + realLen + " targetLen = " +dataLen);
            }
        } catch(IOException e) {
            ArLog.e("Fail to readBytes");
            ArLog.e(e.toString(), e);
            bytes = null;
        }
        return bytes;
    }

    private void writeBeginLength(int len) {
        ByteBuffer byteBuffer = ByteBuffer.allocate(4);
        byteBuffer.order(ByteOrder.nativeOrder());
        byteBuffer.putInt(len);
        try {
            pipeOutputStream.write(byteBuffer.array());
            pipeOutputStream.flush();
        } catch (IOException e) {
            ArLog.e("Fail to write begin length");
            ArLog.e(e.getMessage(), e);
        }
    }

    public void writeBytes(byte[] bytes) {
        int dataLen = bytes.length;
        writeBeginLength(dataLen);
        try {
            pipeOutputStream.write(bytes);
            pipeOutputStream.flush();
        } catch (IOException e) {
            ArLog.e("Fail to write bytes through pipe");
            ArLog.e(e.getMessage(), e);
        }
    }

    public void writeInt(int intData) {
        writeBeginLength(4);
        writeBeginLength(intData);
    }


    private void initPipeFilePath() {
        fromPipeFrontEndPath = pipeDirPath + "/" + fromPipeFrontFileName;
        toPipeFrontEndPath = pipeDirPath + "/" + toPipeFrontFileName;
    }

    private void waitForFileExists(File file) {
        while(!file.exists()) {
            try {
                Thread.sleep(100);
                ArLog.i("No pipe file, sleep 50ms");
            } catch (InterruptedException e) {
                ArLog.e(e.getMessage(), e);
            }
        }
    }

    private void makePipe() {
        File inFile = new File(toPipeFrontEndPath);
        ArLog.i("wait for fifo file" + toPipeFrontEndPath);
        waitForFileExists(inFile);
        File outFile = new File(fromPipeFrontEndPath);
        ArLog.i("wait for fifo file" + fromPipeFrontEndPath);
        waitForFileExists(outFile);

        try {
            FileInputStream fileInputStream = new FileInputStream(inFile);
            BufferedInputStream inputStream = new BufferedInputStream(fileInputStream);
            FileOutputStream fileOutputStream = new FileOutputStream(outFile);
            BufferedOutputStream outputStream = new BufferedOutputStream(fileOutputStream);
            this.pipeInputStream = inputStream;
            this.pipeOutputStream = outputStream;
        } catch (FileNotFoundException e) {
            ArLog.e("Fail to open  pipe File Stream");
            ArLog.e(e.getMessage(), e);
            return;
        }
        ArLog.i("Frontend Finish open pipe");

    }


}
