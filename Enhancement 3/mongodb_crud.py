CronTriggerimport os
from pymongo import MongoClient, errors
from bson.objectid import ObjectId
import logging
from dotenv import load_dotenv
from datetime import datetime
import json
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers.cron import CronTrigger
import atexit

# Load environment variables from .env file
load_dotenv()

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class AnimalShelter:
    # CRUD operations for Animal collection in MongoDB.

    def __init__(self):
        # Initializes the connection to the MongoDB database using environment variables.
        USER = os.getenv('MONGO_USER')
        PASS = os.getenv('MONGO_PASS')
        HOST = os.getenv('MONGO_HOST')
        PORT = int(os.getenv('MONGO_PORT')) 

        # Ensure all necessary environment variables are set
        if not all([USER, PASS, HOST, PORT, DB, COL]):
            logger.error("One or more environment variables are missing.")
            raise EnvironmentError("Missing required environment variables.")

        try:
            # Initialize MongoDB connection
            self.client = MongoClient(f'mongodb://{USER}:{PASS}@{HOST}:{PORT}')
            self.database = self.client[DB]
            self.collection = self.database[COL]
            logger.info("Connected to MongoDB successfully.")
        except Exception as e:
            logger.exception("Failed to connect to MongoDB.")
            raise e

        # Initialize the scheduler for automatic backups
        self.scheduler = BackgroundScheduler()
        self.scheduler.start()
        logger.info("Scheduler started for automatic backups.")

        # Schedule the weekly backup job (e.g., every Sunday at 2 AM)
        trigger = CronTrigger(day_of_week='sun', hour=2, minute=0)
        self.scheduler.add_job(
            self.backup_data,
            trigger,
            args=["weekly_backup.json"],
            id='weekly_backup',
            replace_existing=True
        )
        logger.info("Weekly backup job scheduled: Every Sunday at 2 AM.")

        # Register shutdown_scheduler to be called on program exit
        atexit.register(self.shutdown_scheduler)

    def create(self, data):
        # Inserts a new document into the MongoDB collection after validating required fields.
        # Validate required fields
        required_fields = ['animal_name', 'age', 'breed']
        if not all(field in data for field in required_fields):
            logger.error(f"Missing required fields in data: {required_fields}")
            raise ValueError(f"Missing required fields: {required_fields}")

        if data:
            try:
                result = self.collection.insert_one(data)
                logger.info(f"Inserted document with _id: {result.inserted_id}")
                return result.inserted_id
            except Exception as e:
                logger.exception("Insert operation failed.")
                raise e
        else:
            logger.error("No data provided for insertion.")
            raise ValueError("Data parameter is empty.")

    def read(self, query, limit=10, skip=0):
        # Queries the collection for documents matching the specified criteria with pagination.
        try:
            documents_cursor = self.collection.find(query).skip(skip).limit(limit)
            documents = list(documents_cursor)
            logger.info(f"Retrieved {len(documents)} documents.")
            return documents if documents else []
        except Exception as e:
            logger.exception("Query failed.")
            return []

    def update(self, query, data):
        # Updates documents in the collection matching the specified criteria.
        try:
            result = self.collection.update_many(query, {'$set': data})
            logger.info(f"Modified {result.modified_count} documents.")
            return result.modified_count
        except Exception as e:
            logger.exception("Update operation failed.")
            return 0

    def delete(self, query, soft_delete=False):
        # Deletes or soft deletes documents in the collection that match the specified criteria.
        try:
            if soft_delete:
                result = self.collection.update_many(query, {'$set': {'deleted': True, 'deleted_at': datetime.now()}})
                logger.info(f"Soft deleted {result.modified_count} documents.")
                return result.modified_count
            else:
                result = self.collection.delete_many(query)
                logger.info(f"Deleted {result.deleted_count} documents.")
                return result.deleted_count
        except Exception as e:
            logger.exception("Delete operation failed.")
            return 0

    def enhance_database(self):
        # Enhances the database by adding indexes and optimizing performance.
        try:
            # Add indexes to speed up queries based on breed and age
            self.collection.create_index([('breed', 1)])
            self.collection.create_index([('age_upon_outcome_in_weeks', 1)])
            logger.info("Database indexes added successfully.")
            
            # Additional enhancements can include checking for missing required fields or outdated data
            logger.info("Database enhancement completed.")
        except Exception as e:
            logger.exception("Database enhancement failed.")
            raise e

    def backup_data(self, backup_filename):
        # Backs up the entire collection to a file.
        try:
            # Retrieve all documents from the collection
            documents = list(self.collection.find())
            # Save the documents to a JSON file
            with open(backup_filename, 'w') as f:
                json.dump(documents, f, default=str)  # Convert ObjectId to string
            logger.info(f"Backup completed successfully. {len(documents)} documents backed up to '{backup_filename}'.")
        except Exception as e:
            logger.exception("Backup operation failed.")
            raise e

    def restore_data(self, backup_filename):
        # Restores data from a backup file to the collection.
        try:
            # Read data from the backup file
            with open(backup_filename, 'r') as f:
                documents = json.load(f)
            # Insert documents back into the collection
            self.collection.insert_many(documents)
            logger.info(f"Restored {len(documents)} documents from backup '{backup_filename}'.")
        except Exception as e:
            logger.exception("Restore operation failed.")
            raise e

    def shutdown_scheduler(self):
        # Shuts down the APScheduler scheduler gracefully.
        try:
            self.scheduler.shutdown(wait=False)
            logger.info("Scheduler shut down successfully.")
        except Exception as e:
            logger.exception("Failed to shut down the scheduler.")
            raise e

# Create a new AnimalShelter instance and connect to the database
shelter = AnimalShelter()

# Enhance the database by adding indexes and performing other enhancements
shelter.enhance_database()
