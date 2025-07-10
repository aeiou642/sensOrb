#ifndef VECTOR_HPP
#define VECTOR_HPP

// owen's quick vector implementation because none of the vector libraries were lightweight enough for me to use :)
// will add comments soon

template<typename T>
class vector {
private:
	T* data = nullptr;
	int v_capacity = 1;
    int v_size = 0;
public:
	vector() = default;

	~vector() {
		delete[] this->data;
    this->data = nullptr;
	}

	vector(const vector<T>& other) {
    this->v_size = other.v_size;
    this->v_capacity = other.v_capacity; 
    this->data = new T[v_capacity]; 
    for(int i = 0; i < other.v_size; i++){ 
      this->data[i] = other.data[i];
    };
  }

	vector<T>& operator=(const vector<T>& other) {
    if(this == &other){
      return *this;
    };

    if(this->data != nullptr){
      delete[] this->data;
    };

    this->data = new T[other.v_capacity];
    this->v_capacity = other.v_capacity;
    this->v_size = other.v_size;
    for(int i = 0; i < other.v_size; i++){
      this->data[i] = other.data[i];  
      };
		return *this;
	}

	void push_back(const T& value) {
    this->check_reserves(v_size);
		T* new_data = new T[this->v_capacity];
		for (int i = 0; i < this->v_size; i++) {
			new_data[i] = this->data[i];
		}
		new_data[this->v_size] = value;
		delete [] this->data;
		this->data = new_data;
		this->v_size++;
	}

	T& operator[](int index) {
		return data[index];
	}
	void insert(int index, const T& value) {	
		T* new_data = new T[this->v_capacity];
    this->check_reserves(this->v_size);
    for (int i = 0; i < index; i++) {
			new_data[i] = this->data[i];
		} 
    
    new_data[index] = value; 
    this->v_size++;
    
    for (int i = index + 1; i < this->v_size; i++){
      new_data[i] = this->data[i - 1];

    };	
    
    delete [] this->data;
    this->data = new_data;
  }

  bool check_reserves(int n){
   if(n + 1 > this->v_size){
     reserve(n + 1);
     return true;
    } else {
     return false;
      };
  };

  void reserve(int n){
   if(n < this->v_size){
    // should throw an error here
     } else if(n > this->v_size) {
    T* new_data = new T[this->v_capacity];
    this->v_capacity = this->v_capacity *= 2;
    for(int i = 0; i < this->v_size; i++){
       new_data[i] = this->data[i];
      }; 
    delete[] this->data;
    this->data = new_data;
   } else {
    // should also throw another error here
     };
};

 int getSize(){
    return this->v_size;
 };
};
#endif
