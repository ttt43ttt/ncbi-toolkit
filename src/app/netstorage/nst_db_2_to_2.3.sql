--  $Id: nst_db_2_to_2.3.sql 489145 2016-01-08 19:26:05Z satskyse $
-- ===========================================================================
--
--                            PUBLIC DOMAIN NOTICE
--               National Center for Biotechnology Information
--
--  This software/database is a "United States Government Work" under the
--  terms of the United States Copyright Act.  It was written as part of
--  the author's official duties as a United States Government employee and
--  thus cannot be copyrighted.  This software/database is freely available
--  to the public for use. The National Library of Medicine and the U.S.
--  Government have not placed any restriction on its use or reproduction.
--
--  Although all reasonable efforts have been taken to ensure the accuracy
--  and reliability of the software and data, the NLM and the U.S.
--  Government do not and cannot warrant the performance or results that
--  may be obtained by using this software or data. The NLM and the U.S.
--  Government disclaim all warranties, express or implied, including
--  warranties of performance, merchantability or fitness for any particular
--  purpose.
--
--  Please cite the author in any work or product based on this material.
--
-- ===========================================================================
--
-- Authors:  Sergey Satskiy
--
-- File Description: NetStorage server DB script to update from version 2 to
--                   - DB structure v.2
--                   - SP code v.3
--


-- Changes in the data:
-- - The Versions table now has sp_code record to reflect the version of the
--   stored procedures
-- Changes two stored procedures:
-- - GetStatInfo: to provide SP code version as well
-- - CreateObjectWithClientID: to update record if it already exists



USE [NETSTORAGE];
GO
-- Recommended settings during procedure creation:
SET ANSI_NULLS ON;
SET QUOTED_IDENTIFIER ON;
SET ANSI_PADDING ON;
SET ANSI_WARNINGS ON;
GO


IF 1 = 1
BEGIN
    RAISERROR( 'Fix the condition manually before running the update script', 11, 1 )
    SET NOEXEC ON
END



DECLARE @db_version BIGINT = NULL
SELECT @db_version = version FROM Versions WHERE name = 'db_structure'
IF @@ERROR != 0
BEGIN
    RAISERROR( 'Error retrieving the database structure version', 11, 1 )
    SET NOEXEC ON
END
IF @db_version IS NULL
BEGIN
    RAISERROR( 'Database structure version is not found in the Versions table', 11, 1 )
    SET NOEXEC ON
END
IF @db_version != 2
BEGIN
    RAISERROR( 'Unexpected database structure version. Expected version is 2.', 11, 1 )
    SET NOEXEC ON
END
GO


-- Finally, the DB version is checked, so we can continue


-- Modify the GetStatInfo procedure
ALTER PROCEDURE GetStatInfo
AS
BEGIN
    BEGIN TRANSACTION
        SELECT count(*) AS TotalObjectCount FROM Objects
        SELECT count(*) AS ExpiredObjectCount FROM Objects WHERE tm_expiration IS NOT NULL AND tm_expiration < GETDATE()
        SELECT count(*) AS ClientCount FROM Clients
        SELECT count(*) AS AttributeCount FROM Attributes
        SELECT count(*) AS AttributeValueCount FROM AttrValues
        SELECT version AS DBStructureVersion FROM Versions WHERE name = 'db_structure'
        SELECT version AS SPCodeVersion FROM Versions WHERE name = 'sp_code'
    COMMIT TRANSACTION
    RETURN 0
END
GO


-- Modify the CreateObjectWithClientID procedure
-- See CXX-7719
ALTER PROCEDURE CreateObjectWithClientID
    @object_id          BIGINT,
    @object_key         VARCHAR(256),
    @object_create_tm   DATETIME,
    @object_loc         VARCHAR(256),
    @object_size        BIGINT,
    @client_id          BIGINT,
    @object_expiration  DATETIME
AS
BEGIN
    DECLARE @row_count  INT
    DECLARE @error      INT

    BEGIN TRANSACTION
    BEGIN TRY

        -- Try update first
        UPDATE Objects SET tm_create = @object_create_tm WHERE object_key = @object_key
        SELECT @row_count = @@ROWCOUNT, @error = @@ERROR

        IF @error != 0
        BEGIN
            ROLLBACK TRANSACTION
            RETURN 1
        END

        IF @row_count = 0       -- object is not found; create it
        BEGIN
            INSERT INTO Objects (object_id, object_key, object_loc, client_id, tm_create, size, tm_expiration, read_count, write_count)
            VALUES (@object_id, @object_key, @object_loc, @client_id, @object_create_tm, @object_size, @object_expiration, 0, 1)
            SELECT @row_count = @@ROWCOUNT, @error = @@ERROR

            IF @error != 0 OR @row_count = 0
            BEGIN
                ROLLBACK TRANSACTION
                RETURN 1
            END
        END
    END TRY
    BEGIN CATCH
        ROLLBACK TRANSACTION

        DECLARE @error_message  NVARCHAR(4000) = ERROR_MESSAGE()
        DECLARE @error_severity INT = ERROR_SEVERITY()
        DECLARE @error_state    INT = ERROR_STATE()

        RAISERROR( @error_message, @error_severity, @error_state )
        RETURN 1
    END CATCH

    COMMIT TRANSACTION
    RETURN 0
END
GO






-- The database structure has not changed.
-- However a new component has been introduced: the sp_code
DECLARE @row_count  INT
DECLARE @error      INT
INSERT INTO Versions (name, version) VALUES ('sp_code', 3)
SELECT @row_count = @@ROWCOUNT, @error = @@ERROR
IF @error != 0 OR @row_count = 0
BEGIN
    RAISERROR( 'Cannot insert a stored procedure version (3) into the Versions DB table', 11, 1 )
END
GO

-- Restore if it was changed
SET NOEXEC OFF
GO


