The resource consists of:

* the syncronizer process
* a plugin providing the client-api facade
* a configuration setting of the filters

# Synchronizer
* The synchronization can either:
    * Generate a full diff directly on top of the db. The diffing process can work against a single revision/snapshot (using transactions). It then generates a necessary changeset for the store.
    * If the source supports incremental changes the changeset can directly be generated from that information.

The changeset is then simply inserted in the regular modification queue and processed like all other modifications.
The synchronizer already knows that it doesn't have to replay this changeset to the source, since replay no longer goes via the store.

# Preprocessors
Preprocessors are small processors that are guaranteed to be processed before an new/modified/deleted entity reaches storage. They can therefore be used for various tasks that need to be executed on every entity.

Usecases:

* Update indexes
* Detect spam/scam mail and set appropriate flags
* Email filtering to different folders or resources

The following kinds of preprocessors exist:

* filtering preprocessors that can potentially move an entity to another resource
* passive preprocessors, that extract data that is stored externally (i.e. indexers)
* flag extractors, that produce data stored with the entity (spam detection)

Preprocessors are typically read-only, to i.e. not break signatures of emails. Extra flags that are accessible through the akonadi domain model, can therefore be stored in the local buffer of each resource.

## Requirements
* A preprocessor must work with batch processing. Because batch-processing is vital for efficient writing to the database, all preprocessors have to be included in the batch processing.
* Preprocessors need to be fast, since they directly affect how fast a message is processed by the system.

## Design
Commands are processed in batches. Each preprocessor thus has the following workflow:
* startBatch is called: The preprocessor can do necessary preparation steps to prepare for the batch (like starting a transaction on an external database)
* add/modify/remove is called for every command in the batch: The preprocessor executes the desired actions.
* endBatch is called: If the preprocessor wrote to an external database it can now commit the transaction.

## Indexes
Most indexes are implemented as preprocessors to guarantee that they are always updated together with the data.

Index types:

    * fixed value indexes (i.e. uid)
        * Input: key-value pair where key is the indexed property and the value is the uid of the entity
        * Lookup: by key, value is always zero or more uid's
    * fixed value where we want to do smaller/greater-than comparisons (like start date)
        * Input:
        * Lookup: by key with comparator (greater, equal range)
        * Result: zero or more uid's
    * range indexes (like the date range an event affects)
        * Input: start and end of range and uid of entity
        * Lookup: by key with comparator. The value denotes start or end of range.
        * Result: zero or more uid's
    * group indexes (like tree hierarchies as nested sets)
        * could be the same as fixed value indexes, which would then just require a recursive query.
        * Input:
    * sort indexes (i.e. sorted by date)
        * Could also be a lookup in the range index (increase date range until sufficient matches are available)

### Example index implementations
* uid lookup
    * add:
        * add uid + entity id to index
    * update:
        * remove old uid + entity id from index
        * add uid + entity id to index
    * remove:
        * remove uid + entity id from index
    * lookup:
        * query for entity-id by uid

* mail folder hierarchy
    * parent folder uid is a property of the folder
    * store parent-folder-uid + entity id
    * lookup:
        * query for entity-id by uid

* mails of mail folder
    * parent folder uid is a property of the email
    * store parent-folder-uid + entity id
    * lookup:
        * query for entity-id by uid

* email threads
    * Thread objects should be created as dedicated entities
    * the thread uid

* email date sort index
    * the date of each email is indexed as timestamp

* event date range index
    * the start and end date of each event is indexed as timestamp (floating date-times would change sorting based on current timezone, so the index would have to be refreshed)

### On-demand indexes
To avoid building all indexes initially, and assuming not all indexes are necessarily regularly used for the complete data-set, it should be possible to omit updating an index, but marking it as outdated. The index can then be built on demand when the first query requires the index.

Building the index on-demand is a matter of replaying the relevant dataset and using the usual indexing methods. This should typically be a process that doesn't take too long, and that provides status information, since it will block the query.

The indexes status information can be recorded using the latest revision the index has been updated with.

## Generic Preprocessors
Most preprocessors will likely be used by several resources, and are either completely generic, or domain specific (such as only for mail).
It is therefore desirable to have default implementations for common preprocessors that are ready to be plugged in.

The domain types provide a generic interface to access most properties of the entities, on top of which generic preprocessors can be implemented.
It is that way trivial to i.e. implement a preprocessor that populates a hierarchy index of collections.

## Preprocessors generating additional entities
A preprocessor, such as an email threading preprocessors, might generate additional entities (A thread entity is a regular entity, just like the mail that spawned the thread).

In such a case the preprocessor must invoke the complete pipeline for the new entity.

# Pipeline
A pipeline is an assembly of a set of preprocessors with a defined order. A modification is always persisted at the end of the pipeline once all preprocessors have been processed.