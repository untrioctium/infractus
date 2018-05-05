
/**
 * @brief A class that easily enables any class to be turned
 *        into a global singleton.
 * @tparam T The class that will be managed by the Singleton.
 *
 * Any class that wishes to be utilized by this class must have
 * a default constructor, or else compilation will fail.
 *
 * Using this class is simple; simply pass the class you wish to
 * make a singleton into the \a T template parameter, call the
 * instance function, and then use the class like normal, e.g.:
 * @code
 *	Singleton<MyClass>::instance().myClassMethod();
 * @endcode
 * If you would like a pointer instead, simply get the address
 * using the \c & operator, e.g.:
 * @code
 *	MyClass* singletonPtr = &Singleton<MyClass>::instance();
 * @endcode
 */
template<typename T> class Singleton
{
public:

	/**
	 * @brief Returns a reference to the class singleton.
	 */
	static T& instance()
	{
		if( singleton_ == NULL )
			singleton_ = new T();
			
		return *singleton_;
	}

private:
	static T* singleton_;
};

template<typename T> T* Singleton<T>::singleton_ = NULL;
