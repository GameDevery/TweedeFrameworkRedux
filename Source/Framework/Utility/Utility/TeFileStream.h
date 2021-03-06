#pragma once

#include "Prerequisites/TePrerequisitesUtility.h"
#include <istream>

namespace te
{
    class TE_UTILITY_EXPORT FileStream
    {
    public:
        enum AccessMode
        {
            READ = 1,
            WRITE = 2
        };

    public:
        FileStream();
        FileStream(const String& path, AccessMode mode = READ);
        ~FileStream();

        bool Fail();

        /**
         * Read the requisite number of bytes from the stream, stopping at the end of the file. Advances
         * the read pointer.
         *
         * @param[in]	buf		Pre-allocated buffer to read the data into.
         * @param[in]	count	Number of bytes to read.
         * @return				Number of bytes actually read.
         * 			
         * @note	Stream must be created with READ access mode.
         */
        size_t Read(void* buf, size_t count);

        /**
         * Write the requisite number of bytes to the stream and advance the write pointer.
         *
         * @param[in]	buf		Buffer containing bytes to write.
         * @param[in]	count	Number of bytes to write.
         * @return				Number of bytes actually written.
         * 			
         * @note	Stream must be created with WRITE access mode.
         */
        size_t Write(const void* buf, size_t count);

        /** Returns data as String */
        String GetAsString();

        /**
         * Skip a defined number of bytes. This can also be a negative value, in which case the file pointer rewinds a
         * defined number of bytes.
         */
        void Skip(size_t count);
    
        /** Repositions the read point to a specified byte. */
        void Seek(size_t pos);
        
        /** Returns the current byte offset from beginning. */
        size_t Tell();

        /** Returns true if the stream has reached the end. */
        bool Eof() const;

        /** Closes the file */
        void Close();

        /** Returns the total size of the data to be read from the stream, or 0 if this is indeterminate for this stream. */
        size_t Size() const { return _size; }

        /** Returns the path given in parameter. */
        const String& GetPath() const { return _path; }

        /** Returns the system dependant path computed internally. */
        String GetPlatformPath() const {return  _internalPath; }

        String GetExtension() const { return _extension; }

        virtual bool IsReadable() const { return (_mode & READ) != 0; }
        virtual bool IsWriteable() const { return (_mode & WRITE) != 0; }

    protected:
        void SetInternalPath();
        void SetExtension();
        void CalculteSize();
        void Open();

    protected:
        String _path;
        String _internalPath;
        String _extension;

        AccessMode _mode;

        size_t _size = 0;

        SPtr<std::istream> _inStream;
        SPtr<std::ifstream> _FStreamRO;
        SPtr<std::fstream> _FStream;
    };
}
