#ifndef LIBCOMPUTE_ENGINE_HPP
#define LIBCOMPUTE_ENGINE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

namespace libcompute
{

class Program;

/** Base class for all Engines */
class Engine : public Plugin
{
public:
	/**
	 * @brief Create an instance of a Engine.
	 * @param name The name of the specific Engine.
	 *
	 * Classes that derive from Engine should provide the \a name parameter
	 * in their constructor.
	 */
	Engine( const char* name )
	: Plugin(name, "compute") {}

	/** Class that manages engine specific data storage */
	class DataStorage
	{
	public:
		/** Reference counting shared_ptr for this class. */
		typedef boost::shared_ptr<DataStorage> Ptr;

		/** All possible types for data */
		enum DataType
		{
			Void, ///< Indicates this storage does not actually contain data.
			Int, ///< Indicates this storage is composed of integers.
			Float, ///< Indicates this storage is composed of floats.
			Byte, ///< Indicates this storage is composed of bytes (aka unsigned chars).
		};

		/** Gets the type of this DataStorage.  Usually the owning engine's name. */
		virtual std::string getType() = 0;

		/**
		 * @brief Gets a DataType from a string representation.
		 * @param name The string to convert.
		 * @return The represented DataType.
		 * @throw UnknownTypeException A DataType matching \a name could not be found
		 */
		static DataType typeFromName( const std::string& name ) throw( UnknownTypeException )
		{
			if( dataTypeNameTable_.count(name) == 0 )
				BOOST_THROW_EXCEPTION( UnknownTypeException() << UnknownTypeException::TypeName(name) );
			return dataTypeNameTable_[name];
		}

		/** A small struct that holds information about a DataStorage */
		struct Info
		{
			DataType type; ///< The type of the data contained
			unsigned int size; ///< The number of data items per storage element
			unsigned int width; ///< The width of the data in storage elements
			unsigned int height; ///< The height of the data in storage elements
			unsigned int byteSize; ///< The total size of the data in bytes
			
			bool operator==( const Info& other ) const
			{
				return type == other.type && width == other.width && height == other.height;
			}
		};

		/** Default construtor that sets up the storage to be "filled" */ 
		DataStorage()
		{
			infoSet_ = false;
		}

		/**
		 * @brief Get the Engine specific data location stored by this DataStorage
		 * @return The internal data location.
		 * @throw UninitalizedDataRequestedException The data location was requested before
		 *                                           it was set.
		 *
		 * This method is really only of interest to Engine developers, as a
		 * regular user will copy data back and forth using the toArray() and
		 * fromArray() methods.
		 */
		boost::any getDataStorage() const throw( UninitializedDataRequestedException )
		{
			if( dataStorage_.empty() )
				BOOST_THROW_EXCEPTION( UninitializedDataRequestedException() );
			return dataStorage_;
		}

		/**
		 * @brief Sets the Engine specific data location.
		 * @param The internal data location
		 * @throw DataDoubleSetException An attempt was made to set the storage location
		 *                               when one already existed.
		 *
		 * This method is really only of interest to Engine developers, as a
		 * regular user will copy data back and forth using the toArray() and
		 * fromArray() methods.
		 *
		 * The data storage location can only be set once; attempting to set it again
		 * will result in a DataDoubleSetException.
		 */
		void setDataStorage(boost::any storage) throw( DataDoubleSetException )
		{
			if( !dataStorage_.empty() )
				BOOST_THROW_EXCEPTION( DataDoubleSetException() );
			dataStorage_ = storage;
		}

		/**
		 * @brief Get the Info for this DataStorage.
		 * @return A reference to the Info struct.
		 * @throw UninitalizedDataRequestedException The information was requested before
		 *                                           it was set.
		 */
		const Info& getInfo() const
		{
			if( !infoSet_ )
				BOOST_THROW_EXCEPTION( UninitializedDataRequestedException() );
			return info_;
		}

		/**
		 * @brief Set the Info for this DataStorage.
		 * @param information The Info for this DataStorage.
		 * @throw DataDoubleSetException An attempt was made to set the Information
		 *                               when it already existed.
		 */
		void setInfo( const Info& info )
		{
			if( infoSet_ )
				BOOST_THROW_EXCEPTION( DataDoubleSetException() );
			info_ = info;
			infoSet_ = true;
		}

		/** Destructor overridden by deriving classes so that they appropriately free the stored data */
		virtual ~DataStorage() {}

		/**
		 * @brief Copies an array into the internal data storage.
		 * @param array The array to copy from.
		 *
		 * This method copies an array into the internal storage so that a Engine
		 * can properly utilize it.  Note that the size of the array is unchecked, so
		 * properly assure that the source array's size is either getInformation().byteSizes bytes in size
		 * or has space for getInformation().width * getInformation().height * getInformation().size
		 * elements of the appropriate type.
		 */
		virtual void fromArray( void* array ) = 0;

		/**
		 * @brief Copies the internal data storage into an array.
		 * @param array The array to copy to.
		 *
		 * See the notes for fromArray() for information on how to ensure the array is the
		 * correct size for this operation.
		 */
		virtual void toArray( void* array ) = 0;

	private:

		friend class Information;
		boost::any dataStorage_;
		Info info_;
		bool infoSet_;

		static std::map<std::string, DataType> dataTypeNameTable_;
		static std::map<std::string, DataType> initDataTypeNameTable()
		{
			std::map<std::string, DataType> table;
			table["void"] = Void;
			table["int"] = Int;
			table["float"] = Float;
			table["byte"] = Byte;
			return table;
		}

		static std::map<DataType, unsigned int> dataTypeSizeTable_;
		static std::map<DataType, unsigned int> initDataTypeSizeTable()
		{
			std::map<DataType, unsigned int> table;
			table[Int] = sizeof(int);
			table[Float] = sizeof(float);
			table[Byte] = sizeof(unsigned char);
			return table;
		}
	};


	virtual void* bindProgram( Program* const ) = 0;
	virtual void unbindProgram( Program* const) = 0;

	virtual void runProgram( Program* const) = 0;
	
	enum ReductionType
	{
		Minimum,
		Maximum,
		Sum,
	};
	
	virtual vec4 reduce( const DataStorage::Ptr& storage, ReductionType type ) = 0;

	virtual DataStorage::Ptr allocateStorage( const DataStorage::Info& type, int width, int height ) = 0;
	virtual DataStorage::Ptr emptyStorage() = 0;
};

};

#endif
