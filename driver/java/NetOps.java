
/*
 * File:  NetOps.java
 * Copyright (C) 2004 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 */


package ru.ispras.sedna.driver;

//~--- JDK imports ------------------------------------------------------------

import java.io.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.lang.*;

import java.net.Socket;

import java.nio.*;
import java.nio.charset.*;

//~--- classes ----------------------------------------------------------------

/**
 * Static functions to organize message exchange between
 * client application and kernel
 */
class NetOps {
    static Object    currentStatement          = null;
    final static int SEDNA_SOCKET_MSG_BUF_SIZE = 10240;

    /**
     * Size of bulk load portion
     */
    final static int SEDNA_BULK_LOAD_PORTION           = 5120;
    final static int se_ErrorResponse                  = 100;
    final static int se_StartUp                        = 110;
    final static int se_SessionParameters              = 120;
    final static int se_SendSessionParameters          = 140;
    final static int se_SendAuthParameters             = 150;
    final static int se_RollbackTransactionOk          = 255;
    final static int se_RollbackTransactionFailed      = 265;
    final static int se_RollbackTransaction            = 225;
    final static int se_QuerySucceeded                 = 320;
    final static int se_QueryFailed                    = 330;
    final static int se_LongQueryEnd                   = 302;
    final static int se_GetNextItem                    = 310;
    final static int se_ExecuteLong                    = 301;
    final static int se_Execute                        = 300;
    final static int se_CommitTransactionOk            = 250;
    final static int se_CommitTransactionFailed        = 260;
    final static int se_CommitTransaction              = 220;
    final static int se_BeginTransactionOk             = 230;
    final static int se_BeginTransactionFailed         = 240;
    final static int se_BeginTransaction               = 210;
    final static int se_AuthenticationParameters       = 130;
    final static int se_AuthenticationOK               = 160;
    final static int se_AuthenticationFailed           = 170;
    final static int se_UpdateSucceeded                = 340;
    final static int se_UpdateFailed                   = 350;
    final static int se_TransactionRollbackBeforeClose = 520;
    final static int se_ShowTime                       = 451;
    final static int se_ResultEnd                      = 375;
    final static int se_LastQueryTime                  = 452;
    final static int se_ItemPart                       = 360;
    final static int se_ItemEnd                        = 370;
    final static int se_ExecuteSchemeProgram           = 95;
    final static int se_CloseConnectionOk              = 510;
    final static int se_CloseConnection                = 500;
    final static int se_BulkLoadSucceeded              = 440;
    final static int se_BulkLoadPortion                = 410;
    final static int se_BulkLoadFromStream             = 431;
    final static int se_BulkLoadFileName               = 430;
    final static int se_BulkLoadFailed                 = 450;
    final static int se_BulkLoadError                  = 400;
    final static int se_BulkLoadEnd                    = 420;
    final static int se_Authenticate                   = 90;
//    static byte      int_array[]                       = new byte[4];

    //~--- methods ------------------------------------------------------------

    /**
     * Loads data from stream to Server via sockets
     * false - if bulk Load (it is update) succeeded
     * true - if failed
     */
   static boolean bulkLoad(InputStream in,
                            BufferedInputStream bufInputStream,
                            OutputStream outputStream)
            throws DriverException {
        NetOps.Message msg        = new NetOps.Message();
        int            bytes_read = 0;

        try {
            while (bytes_read != -1) {
                bytes_read = in.read(msg.body, 5, SEDNA_BULK_LOAD_PORTION);

                if (bytes_read != -1) {
                    msg.instruction = 410;    // BulkLoadPortion
                    msg.length      = bytes_read + 5;
                    msg.body[0]     = 0;
                    NetOps.writeInt(bytes_read, msg.body, 1);
                    NetOps.writeMsg(msg, outputStream);
                }
            }
        } catch (IOException ioe) {
            msg.instruction = 400;            // BulkLoadError
            msg.length      = 0;
            NetOps.writeMsg(msg, outputStream);
            NetOps.readMsg(msg, bufInputStream);

            throw new DriverException(ErrorCodes.SE3007, ioe.toString());            
        } catch (DriverException de) {
            msg.instruction = 400;            // BulkLoadError
            msg.length      = 0;
            NetOps.writeMsg(msg, outputStream);
            NetOps.readMsg(msg, bufInputStream);

            throw de;
        }

        msg.instruction = 420;    // BulkLoadEnd
        msg.length      = 0;
        NetOps.writeMsg(msg, outputStream);

        // result of bulk loading
        NetOps.readMsg(msg, bufInputStream);

        if ((msg.instruction == 440) || (msg.instruction == 340)) {
            return false;
        } else if ((msg.instruction == 450) || (msg.instruction == 350)
                   || (msg.instruction == 100)) {
            throw new DriverException(getErrorInfo(msg.body, msg.length),getErrorCode(msg.body));
        } else {
            throw new DriverException(ErrorCodes.SE3008, "");
        }
    }

    static void driverErrOut(String str) {
        if (Debug.DEBUG) {
            System.err.println(str);
        }
    }

    static void driverPrintOut(String str) {
        if (Debug.DEBUG) {
            System.err.print(str);
        }
    }

    static int readInt(BufferedInputStream bufInputStream)
            throws DriverException {
        int call_res, integer = 0;
        byte int_array[] = new byte[4];

        try {
	        call_res = bufInputStream.read(int_array, 0, 4);
    	    if (call_res != 4) throw new DriverException(ErrorCodes.SE3007, "");
        
        	integer = (((int_array[0] & 0xff) << 24)
                       | ((int_array[1] & 0xff) << 16)
                       | ((int_array[2] & 0xff) << 8) | (int_array[3] & 0xff));
        }catch (IOException ioe) {
            throw new DriverException(ErrorCodes.SE3007, ioe.toString());
        }
             
        return integer;
    }

    static void readMsg(Message msg, BufferedInputStream bufInputStream)
            throws DriverException {
        int call_res;

        try {
            msg.instruction = NetOps.readInt(bufInputStream);
            msg.length      = NetOps.readInt(bufInputStream);
            if (msg.length > SEDNA_SOCKET_MSG_BUF_SIZE) {
                throw new DriverException(ErrorCodes.SE3012, "");
            }

            if (msg.length != 0) {
                int count = 0;
                int pos   = 0;

                while (pos < msg.length) {
                    count = bufInputStream.read(msg.body, pos,
                                                msg.length - pos);

                    if (count != -1) {
                        pos += count;
                    }
                }
            }
        } catch (IOException ioe) {
            throw new DriverException(ErrorCodes.SE3007, ioe.toString());
        }
    }

    /**
     *  Reads a whole item from the socket
     */
    static String_item readStringItem(BufferedInputStream is)
            throws DriverException {
        NetOps.Message     msg   = new NetOps.Message();
        NetOps.String_item sitem = new NetOps.String_item();

        sitem.item = new StringBuffer();

        ByteBuffer     byteBuf;
        CharBuffer     charBuf =
            CharBuffer.allocate(SEDNA_SOCKET_MSG_BUF_SIZE);
        CharsetDecoder csd     = Charset.forName("utf8").newDecoder();

        // StringBuffer strBuf = new StringBuffer();

        NetOps.readMsg(msg, is);

        if ((msg.instruction == 370) || (msg.instruction == 375))    // ItemEnd or ResultEnd
        {
            sitem.hasNextItem = false;
            sitem.item        = null;

            return sitem;
        }

        while ((msg.instruction != 370) && (msg.instruction != 375)) {
            if (msg.instruction == 100) {    // ErrorResponse
                throw new DriverException(NetOps.getErrorInfo(msg.body, msg.length), NetOps.getErrorCode(msg.body));
            }

            if (msg.instruction == 360)    // ItemPart
            {
                byteBuf = ByteBuffer.wrap(msg.body, 5, msg.length - 5);
                csd.decode(byteBuf, charBuf, false);

                // strBuf.append(charBuf.flip());
                sitem.item.ensureCapacity(charBuf.length());

                try {
                    sitem.item.append(charBuf.flip());
                } catch (OutOfMemoryError e) {}

                charBuf.clear();
            }

            NetOps.readMsg(msg, is);
        }

        if (msg.instruction == 375) {
            sitem.hasNextItem = false;
        }

        if (msg.instruction == 370) {
            sitem.hasNextItem = true;
        }

        return sitem;
    }

    static void writeInt(int i, BufferedOutputStream bufOutputStream)
            throws IOException {
        bufOutputStream.write(0xff & (i >> 24));
        bufOutputStream.write(0xff & (i >> 16));
        bufOutputStream.write(0xff & (i >> 8));
        bufOutputStream.write(0xff & i);
    }

    static void writeInt(int i, byte[] byte_array, int pos) {
        byte_array[pos]     = (new Integer(0xff & (i >> 24))).byteValue();
        byte_array[pos + 1] = (new Integer(0xff & (i >> 16))).byteValue();
        byte_array[pos + 2] = (new Integer(0xff & (i >> 8))).byteValue();
        byte_array[pos + 3] = (new Integer(0xff & i)).byteValue();
    }

    static void writeMsg(Message msg, OutputStream outputStream)
            throws DriverException {
        if (msg.length > SEDNA_SOCKET_MSG_BUF_SIZE) {
            throw new DriverException(ErrorCodes.SE3012, "");
        }

        BufferedOutputStream bufOutputStream =
            new BufferedOutputStream(outputStream);

        try {
            NetOps.writeInt(msg.instruction, bufOutputStream);
            NetOps.writeInt(msg.length, bufOutputStream);

            if (msg.length != 0) {
                bufOutputStream.write(msg.body, 0, msg.length);
            }

            bufOutputStream.flush();
        } catch (IOException ioe) {
            throw new DriverException(ErrorCodes.SE3006, "");
        }
    }

    //~--- get methods --------------------------------------------------------

    /**
     *  Gets error message body and
     *  Makes a string that is error info (usually for DriverException)
     */
    static String getErrorInfo(byte[] body, int length) {
        return new String(body, 9, length - 9);
    }

    /**
     *  Gets error code
     */
    static int getErrorCode(byte[] body) {
        
        int integer = (((body[0] & 0xff) << 24)
                     | ((body[1] & 0xff) << 16)
                     | ((body[2] & 0xff) << 8) 
                     | (body[3] & 0xff));    	
        return integer;
    }

    //~--- inner classes ------------------------------------------------------

    static class Message {
        byte body[];
        int  instruction;
        int  length;

        //~--- constructors ---------------------------------------------------

        Message() {
            this.body = new byte[SEDNA_SOCKET_MSG_BUF_SIZE];
        }
    }


    static class String_item {
        boolean      hasNextItem;
        StringBuffer item;
    }
}    // end of NetOps class
