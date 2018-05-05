#include <stdlib.h>

template<typename T> class Array1D
{
public:
	Array1D(int size)
	: arraySize_(size)
	, array_((T*) malloc(sizeof(T) * size)) {}

	~Array1D() { free(array_); }
	int size() { return arraySize_; }
	T& getRef( int index ) { return array_[index]; }
	T* getPtr( int index ) { return &getRef(index); }
	void set( int index, T value ) { array_[index] = value; }
	void setAll( T value ) { for( int i = 0; i < arraySize_; i++ ) array_[i] = value; }

	T get( int index ) { return array_[index]; }
	T& operator[]( int index ) { return getRef(index); }
	T& operator()( int index ) { return getRef(index); }

	T* array() { return array_; }
private:
	int arraySize_;

	T* array_;
};

template<typename T> class Array2D
{
public:
	Array2D( int width, int height )
	: width_(width)
	, height_(height)
	, array_( new T[ width * height ] ) {}

	~Array2D()
	{
		delete[] array_;
	}

	int width() { return width_; }
	int height() { return height_; }

	T get( int x, int y )
	{
		return array_[ y * width_ + x ];
	}

	T& getRef( int x, int y )
	{
		return array_[y * width_ + x];
	}

	T* getPtr( int x, int y )
	{
		return &getRef(x, y);
	}

	void set( int x, int y, T value )
	{
		array_[y * width_ + x] = value;
	}

	T& operator()( int x, int y )
	{
		return array_[y * width_ + x];
	}

	T* array() { return array_; }

private:

	int width_, height_;
	
	T* array_;
};
