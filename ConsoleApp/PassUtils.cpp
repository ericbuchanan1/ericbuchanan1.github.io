#include "PassUtils.h"

#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>
#include <iostream>
#include <random>
#include <stdexcept>
namespace PassUtils {

    std::string generateSalt() {
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::byte salt[16];
        prng.GenerateBlock(salt, sizeof(salt));
        return std::string(reinterpret_cast<char*>(salt), sizeof(salt));
    }

    std::string hashPasswordPBKDF2(const std::string& password, const std::string& salt) {
        CryptoPP::SecByteBlock derived(32); // 256-bit key
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;
        pbkdf2.DeriveKey(
            derived, derived.size(),
            0,
            reinterpret_cast<const CryptoPP::byte*>(password.data()), password.size(),
            reinterpret_cast<const CryptoPP::byte*>(salt.data()), salt.size(),
            10000 // Number of iterations
        );

        std::string encoded;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(encoded));
        encoder.Put(derived, derived.size());
        encoder.MessageEnd();
        return encoded;
    }

    bool verifyPasswordPBKDF2(const std::string& password, const std::string& salt, const std::string& hash) {
        std::string computedHash = hashPasswordPBKDF2(password, salt);
        return computedHash == hash;
    }
   
    /**
     * @brief Generates a random password using specified allowed characters.
     *
     * This function creates a password composed of uppercase letters, lowercase letters,
     * digits, and specified special symbols to ensure complexity and security.

     * @return std::string The generated password.
     */
    
    std::string generateRandomPassword() {
        // Default Password Length
        const int PASSWORD_LENGTH = 16;

        const std::string lowercase = "abcdefghijklmnopqrstuvwxyz";
        const std::string uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const std::string digits = "0123456789";
        const std::string specials = "!@#$%^&*()-_=+";
        const std::string allowed_chars = lowercase + uppercase + digits + specials;

        std::random_device rd;
        std::mt19937 gen(rd());

        // Use uniform distribution to randomly select characters from each character set
		std::uniform_int_distribution<size_t> lower_dist(0, lowercase.size() - 1);
		std::uniform_int_distribution<size_t> upper_dist(0, uppercase.size() - 1);
		std::uniform_int_distribution<size_t> digit_dist(0, digits.size() - 1);
		std::uniform_int_distribution<size_t> special_dist(0, specials.size() - 1);
		std::uniform_int_distribution<size_t> all_dist(0, allowed_chars.size() - 1);


        // Generate at least one character from each category
        std::string password;
        password += lowercase[lower_dist(gen)];
        password += uppercase[upper_dist(gen)];
        password += digits[digit_dist(gen)];
        password += specials[special_dist(gen)];

        // Generate remaining characters
        for (int i = 4; i < PASSWORD_LENGTH; ++i) {
            password += allowed_chars[all_dist(gen)];
        }

        // Shuffle the password to mix the characters
        std::shuffle(password.begin(), password.end(), gen);

        std::cout << "New Randomized Password is: " << password;

        return password;
    }
}
