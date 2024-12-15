package com.example.androidapp.utils;
import at.favre.lib.crypto.bcrypt.BCrypt;


public class passUtils {
    public static String hashPassword(String plaintextPassword){
        return BCrypt.withDefaults().hashToString(12, plaintextPassword.toCharArray());
    }
    public static boolean verifyPassword(String plaintextPassword, String hashedPassword){
        BCrypt.Result result = BCrypt.verifyer().verify(plaintextPassword.toCharArray(), hashedPassword);
        return result.verified;
    }
}
