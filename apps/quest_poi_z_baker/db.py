"""Small MySQL helper for the quest POI Z baker.

The acore-data baker used ``core.api.sql``, which pulls in the whole data
tooling stack. The baker only needs the world database, so this keeps the tool
self-contained with a single pymysql connection and the same dict-row shape.

Connection defaults match a local AzerothCore install; override them with the
``ACORE_DB_*`` environment variables or the ``configure()`` call from the CLI.
"""

from __future__ import annotations

import os
from typing import Any, Dict, List, Optional, Sequence

try:
    import pymysql
    from pymysql.cursors import DictCursor
except ImportError:  # pure baker tests do not need a database
    pymysql = None
    DictCursor = None

DEFAULTS = {
    "host": os.environ.get("ACORE_DB_HOST", "127.0.0.1"),
    "port": int(os.environ.get("ACORE_DB_PORT", "3306")),
    "user": os.environ.get("ACORE_DB_USER", "acore"),
    "password": os.environ.get("ACORE_DB_PASSWORD", os.environ.get("ACORE_DB_PASS", "acore")),
    "database": os.environ.get("ACORE_DB_NAME", "acore_world"),
    "charset": os.environ.get("ACORE_DB_CHARSET", "utf8mb4"),
}

_settings = dict(DEFAULTS)
_connection: Optional[pymysql.connections.Connection] = None


def configure(**kwargs: Any) -> None:
    """Override connection settings before the first query."""
    global _connection
    for key, value in kwargs.items():
        if value is not None:
            _settings[key] = value
    if _connection is not None:
        _connection.close()
        _connection = None


def _connect() -> pymysql.connections.Connection:
    if pymysql is None:
        raise RuntimeError(
            "pymysql is required to bake; install apps/quest_poi_z_baker/requirements.txt"
        )
    return pymysql.connect(
        host=_settings["host"],
        port=int(_settings["port"]),
        user=_settings["user"],
        password=_settings["password"],
        database=_settings["database"],
        charset=_settings["charset"],
        cursorclass=DictCursor,
        autocommit=True,
    )


def sql(query: str, params: Sequence[Any] = ()) -> List[Dict[str, Any]]:
    """Run a query against the world database and return dict rows."""
    global _connection
    if _connection is None:
        _connection = _connect()
    with _connection.cursor() as cursor:
        cursor.execute(query, params)
        return list(cursor.fetchall())


def close() -> None:
    global _connection
    if _connection is not None:
        _connection.close()
        _connection = None
