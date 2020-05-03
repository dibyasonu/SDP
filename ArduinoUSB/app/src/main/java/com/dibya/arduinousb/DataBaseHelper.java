package com.dibya.arduinousb;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DataBaseHelper extends SQLiteOpenHelper {

    // Database Version
    private static final int DATABASE_VERSION = 1;
    private AttendanceDetails attendanceDetails;
    // Database Name
    private static final String DATABASE_NAME = "attendance.db";


    public DataBaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    // Creating Tables
    @Override
    public void onCreate(SQLiteDatabase db) {
        // create notes table
        db.execSQL(AttendanceDetails.CREATE_TABLE);
    }

    // Upgrading database
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // Drop older table if existed
        db.execSQL("DROP TABLE IF EXISTS " + AttendanceDetails.TABLE_NAME);
        // Create tables again
        onCreate(db);
    }

    public long enrollStudent(int id, String sec, String branch, String fp_packet) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues cv = new ContentValues();
        cv.put("id", id);
        cv.put("section", sec);
        cv.put("branch", branch);
        cv.put("fppacket", fp_packet);

        long res = db.insert("attendance", null, cv);
        db.close();
        return res;
    }

    public AttendanceDetails[] getData(String branch, String sec) {
        SQLiteDatabase db = this.getReadableDatabase();
        final String TABLE_NAME = "attendance";
        String selectQuery = "SELECT * FROM " + TABLE_NAME + " WHERE branch =" + "'" + branch + "'" + " AND section =" + "'" + sec + "'";
//        String selectQuery = "SELECT * FROM " + TABLE_NAME;
        Log.d("Query", selectQuery);
        Cursor cursor = db.rawQuery(selectQuery, null);
        AttendanceDetails [] data = new AttendanceDetails[cursor.getCount()];
        int i = 0;
        if (cursor.moveToFirst()) {
            do {
                attendanceDetails = new AttendanceDetails(
                        cursor.getInt(cursor.getColumnIndex(AttendanceDetails.STUDENT_ID)),
                        cursor.getString(cursor.getColumnIndex(AttendanceDetails.CLASS_NAME)),
                        cursor.getString(cursor.getColumnIndex(AttendanceDetails.BRANCH)),
                        cursor.getString(cursor.getColumnIndex(AttendanceDetails.FP_PACKET)));
                data[i] = attendanceDetails;
                Log.d("details",attendanceDetails.getStudent_branch());
                Log.d("details",attendanceDetails.getStudent_sec());
            } while (cursor.moveToNext());
            cursor.close();
        }
        return data;
    }
}