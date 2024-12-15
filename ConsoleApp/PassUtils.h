// PassUtils.h
#ifndef PASSUTILS_H
#define PASSUTILS_H
#pragma once
#include <string>

namespace PassUtils {

	/**
	 * @brief Generates a random salt for password hashing.
	 *
	 * @return std::string The generated salt.
	 */
	std::string generateSalt();

	/**
	 * @brief Hashes a password using PBKDF2 with the provided salt.
	 *
	 * @param password The plaintext password.
	 * @param salt The salt.
	 * @return std::string The hashed password in hexadecimal format.
	 */
	std::string hashPasswordPBKDF2(const std::string& password, const std::string& salt);

	/**
	 * @brief Verifies a password against the stored hash.
	 *
	 * @param password The plaintext password to verify.
	 * @param salt The salt used during hashing.
	 * @param hash The stored hashed password.
	 * @return true If the password is correct.
	 * @return false If the password is incorrect.
	 */
	bool verifyPasswordPBKDF2(const std::string& password, const std::string& salt, const std::string& hash);

	/**
	 * @brief Generates a random password using specified allowed characters.
	 *
	 * @param length The desired length of the password.
	 * @return std::string The generated password.
	 */
	std::string generateRandomPassword();

} // namespace PassUtils

#endif // PASSUTILS_H
