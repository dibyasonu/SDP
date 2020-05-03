package com.dibya.arduinousb;

public class AttendanceDetails {
    private int op;
    private int id;
    private String fp_packet;
    private String student_name;
    private String student_branch;
    private String student_sec;
    public static final String TABLE_NAME = "attendance";
    public static final String STUDENT_ID = "id";
    public static final String FP_PACKET = "fppacket";
    public static final String CLASS_NAME = "section";
    public static final String BRANCH = "branch";

    public static final String CREATE_TABLE =
            "CREATE TABLE " + TABLE_NAME + "("
                    + STUDENT_ID + " INTEGER PRIMARY KEY,"
                    + CLASS_NAME + " TEXT,"
                    + BRANCH + " TEXT,"
                    + FP_PACKET + " TEXT"
                    + ")";

    public AttendanceDetails(int id, String section, String branch, String fp_packet) {
        this.id = id;
        this.student_sec = section;
        this.student_branch = branch;
        this.fp_packet = fp_packet;
    }

    public int getOp() {
        return op;
    }

    public void setOp(int op) {
        this.op = op;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getFp_packet() {
        return fp_packet;
    }

    public void setFp_packet(String fp_packet) {
        this.fp_packet = fp_packet;
    }

    public String getStudent_name() {
        return student_name;
    }

    public void setStudent_name(String student_name) {
        this.student_name = student_name;
    }

    public String getStudent_branch() {
        return student_branch;
    }

    public void setStudent_branch(String student_branch) {
        this.student_branch = student_branch;
    }

    public String getStudent_sec() {
        return student_sec;
    }

    public void setStudent_sec(String student_sec) {
        this.student_sec = student_sec;
    }

}
