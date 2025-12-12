# Using synchronous and wal_autocheckpoint Options

## Problem
Previously, trying to set `PRAGMA synchronous` would fail because DuckDB wraps all queries in transactions:

```sql
-- This would fail with: "Safety level may not be changed inside a transaction"
SELECT sqlite_query('mydb', 'PRAGMA synchronous = 1');
```

## Solution
Set these pragmas during ATTACH, before any transactions start:

### Example 1: Set synchronous mode

```sql
-- Attach with synchronous=OFF for maximum performance (no disk syncs)
ATTACH 'mydata.db' AS mydb (TYPE sqlite, synchronous='OFF');

-- Or use other values: 'NORMAL', 'FULL', 'EXTRA'
ATTACH 'mydata.db' AS mydb (TYPE sqlite, synchronous='NORMAL');
```

### Example 2: Control WAL autocheckpoint

```sql
-- Disable automatic WAL checkpointing
ATTACH 'mydata.db' AS mydb (TYPE sqlite, journal_mode='WAL', wal_autocheckpoint=0);

-- Set checkpoint threshold to 5000 pages instead of default 1000
ATTACH 'mydata.db' AS mydb (TYPE sqlite, journal_mode='WAL', wal_autocheckpoint=5000);
```

### Example 3: Combine multiple options

```sql
-- High-performance setup: WAL mode, minimal syncing, infrequent checkpoints
ATTACH 'mydata.db' AS mydb (
    TYPE sqlite,
    journal_mode='WAL',
    synchronous='NORMAL',
    wal_autocheckpoint=10000
);
```

## Performance Impact

### synchronous modes:
- **OFF**: Fastest, no disk syncs (risk of corruption on power loss)
- **NORMAL**: Good balance (WAL mode syncs only at checkpoints)
- **FULL**: Safer, syncs after every commit (slower)
- **EXTRA**: Maximum safety (slowest)

### wal_autocheckpoint:
- **0**: Disable automatic checkpointing (WAL grows indefinitely)
- **1000** (default): Checkpoint after 1000 pages
- **Higher values**: Less frequent checkpoints, better insert performance

## Verification

You can verify the settings are applied:

```sql
-- Check synchronous mode
SELECT sqlite_query('mydb', 'PRAGMA synchronous');

-- Check wal_autocheckpoint
SELECT sqlite_query('mydb', 'PRAGMA wal_autocheckpoint');
```

## Notes

- These pragmas MUST be set during ATTACH, not after
- They are set before any transactions start, avoiding the "inside a transaction" error
- These settings apply to the entire attached database connection
