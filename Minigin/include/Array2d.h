#pragma once

#include <vector>
#include <stdexcept>



template<typename T>
class Array2d
{
    private:

    std::vector<T> m_Data;
    size_t m_Rows;
    size_t m_Cols;



    public:

    Array2d() : m_Data(), m_Rows(0), m_Cols(0) { }

    Array2d(size_t rows, size_t cols, const T& value = T())
        : m_Data(rows* cols, value), m_Rows(rows), m_Cols(cols)
    { }

    Array2d(size_t rows, size_t cols, std::vector<T>&& data)
        : m_Data(data), m_Rows(rows), m_Cols(cols)
    { }

    Array2d(std::initializer_list<std::initializer_list<T>> initList)
    {
        m_Rows = initList.size();
        m_Cols = initList.begin()->size();
        m_Data.reserve(m_Rows * m_Cols);

        for (const auto& row : initList)
        {
            if (row.size() != m_Cols)
            {
                throw std::invalid_argument("All rows must have the same number of columns.");
            }

            m_Data.insert(m_Data.end(), row.begin(), row.end());
        }
    }

    Array2d(const Array2d& other)
        : m_Data(other.m_Data), m_Rows(other.m_Rows), m_Cols(other.m_Cols)
    { }

    Array2d& operator=(const Array2d& other)
    {
        if (this != &other)
        {
            m_Data = other.m_Data;
            m_Rows = other.m_Rows;
            m_Cols = other.m_Cols;
        }

        return *this;
    }

    Array2d(Array2d&& other) noexcept
        : m_Data(std::move(other.m_Data)), m_Rows(other.m_Rows), m_Cols(other.m_Cols)
    {
        other.m_Rows = 0;
        other.m_Cols = 0;
    }

    Array2d& operator=(Array2d&& other) noexcept
    {
        if (this != &other)
        {
            m_Data = std::move(other.m_Data);
            m_Rows = other.m_Rows;
            m_Cols = other.m_Cols;

            other.m_Rows = 0;
            other.m_Cols = 0;
        }

        return *this;
    }

    decltype(auto) operator()(size_t row, size_t col)
    {
        if (row >= m_Rows || col >= m_Cols)
            throw std::out_of_range("Array2d index out of range");

        return m_Data[row * m_Cols + col];
    }

    decltype(auto) operator()(size_t index)
    {
        if (index >= m_Data.size())
            throw std::out_of_range("Array2d index out of range");

        return m_Data[index];
    }

    decltype(auto) operator()(size_t row, size_t col) const
    {
        if (row >= m_Rows || col >= m_Cols)
            throw std::out_of_range("Array2d index out of range");

        return m_Data[row * m_Cols + col];
    }

    decltype(auto) operator()(size_t index) const
    {
        if (index >= m_Data.size())
            throw std::out_of_range("Array2d index out of range");

        return m_Data[index];
    }

    void Resize(size_t newRows, size_t newCols, const T& value = T())
    {
        m_Rows = newRows;
        m_Cols = newCols;
        m_Data.assign(m_Rows * m_Cols, value);
    }

    size_t Rows() const { return m_Rows; }
    size_t Cols() const { return m_Cols; }
    size_t Size() const { return m_Data.size(); }

    /* Returns the flat id in the vector container */
    size_t Id(size_t row, size_t col) const { return row * m_Cols + col; }

    std::vector<T>& GetData() { return m_Data; }
};