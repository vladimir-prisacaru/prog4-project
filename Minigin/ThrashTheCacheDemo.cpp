#include <chrono>
#include <algorithm>
#include <numeric>
#include <imgui.h>
#include <implot.h>
#include "ThrashTheCacheDemo.h"



void dae::ThrashTheCacheDemo::Update(float)
{
    ImGui::Begin("Thrash The Cache");

    ImGui::InputInt("Sample Size", &m_SampleSize, 1000, 10000);
    ImGui::InputInt("Number of Samples", &m_NumSamples, 1, 10);

    m_SampleSize = std::clamp(m_SampleSize, 1, 1'000'000);
    m_NumSamples = std::clamp(m_NumSamples, 1, 100);

    if (ImGui::Button("Run Benchmark 1"))
    {
        RunBenchmark1(m_SampleSize);
    }

    if (ImGui::Button("Run Benchmark 2"))
    {
        RunBenchmark2(m_SampleSize);
    }

    if (ImGui::Button("Run Benchmark 3"))
    {
        RunBenchmark3(m_SampleSize);
    }

    ImGui::Separator();

    // Plot 1
    if (!m_Strides1.empty() && !m_Results1.empty())
    {
        assert(m_Strides1.size() == m_Results1.size());

        if (ImPlot::BeginPlot("Benchmark 1 - int"))
        {
            ImPlot::SetupAxis(ImAxis_X1, "Stride");

            ImPlot::SetupAxis(ImAxis_Y1, "Time (ms)");

            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);

            ImPlot::PlotLine("",
                m_Strides1.data(),
                m_Results1.data(),
                static_cast<int>(m_Results1.size()));

            ImPlot::EndPlot();
        }
    }

    // Plot 2
    if (!m_Strides2.empty() && !m_Results2.empty())
    {
        assert(m_Strides2.size() == m_Results2.size());

        if (ImPlot::BeginPlot("Benchmark 2 - 68 byte struct"))
        {
            ImPlot::SetupAxis(ImAxis_X1, "Stride");

            ImPlot::SetupAxis(ImAxis_Y1, "Time (ms)");

            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);

            ImPlot::PlotLine("",
                m_Strides2.data(),
                m_Results2.data(),
                static_cast<int>(m_Results2.size()));

            ImPlot::EndPlot();
        }
    }

    // Plot 3
    if (!m_Strides3.empty() && !m_Results3.empty())
    {
        assert(m_Strides3.size() == m_Results3.size());

        if (ImPlot::BeginPlot("Benchmark 3 - 16 byte struct"))
        {
            ImPlot::SetupAxis(ImAxis_X1, "Stride");

            ImPlot::SetupAxis(ImAxis_Y1, "Time (ms)");

            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);

            ImPlot::PlotLine("",
                m_Strides3.data(),
                m_Results3.data(),
                static_cast<int>(m_Results3.size()));

            ImPlot::EndPlot();
        }
    }

    ImGui::End();
}

void dae::ThrashTheCacheDemo::RunBenchmark1(int size)
{
    m_Results1.clear();
    m_Strides1.clear();

    const int numStrides { 11 };
    std::vector<std::vector<double>> samples(numStrides);

    for (int run = 0; run < m_NumSamples; ++run)
    {
        std::vector<int> arr(size);

        int strideIdx = 0;
        for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
        {
            auto start { std::chrono::high_resolution_clock::now() };

            for (int i = 0; i < size; i += stride)
                arr[i] *= 2;

            auto end { std::chrono::high_resolution_clock::now() };

            samples[strideIdx].push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
    }

    int strideIdx { 0 };
    for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
    {
        m_Strides1.push_back(static_cast<double>(stride));
        m_Results1.push_back(AverageWithoutOutliers(samples[strideIdx]));
    }
}

void dae::ThrashTheCacheDemo::RunBenchmark2(int size)
{
    m_Results2.clear();
    m_Strides2.clear();

    const int numStrides { 11 };
    std::vector<std::vector<double>> samples(numStrides);

    for (int run = 0; run < m_NumSamples; ++run)
    {
        std::vector<gameobject1> arr(size);

        int strideIdx = 0;
        for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
        {
            auto start { std::chrono::high_resolution_clock::now() };

            for (int i = 0; i < size; i += stride)
                arr[i].id *= 2;

            auto end { std::chrono::high_resolution_clock::now() };

            samples[strideIdx].push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
    }

    int strideIdx { 0 };
    for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
    {
        m_Strides2.push_back(static_cast<double>(stride));
        m_Results2.push_back(AverageWithoutOutliers(samples[strideIdx]));
    }
}

void dae::ThrashTheCacheDemo::RunBenchmark3(int size)
{
    m_Results3.clear();
    m_Strides3.clear();

    const int numStrides = 11;
    std::vector<std::vector<double>> samples(numStrides);

    for (int run = 0; run < m_NumSamples; ++run)
    {
        std::vector<gameobject2> arr(size);

        int strideIdx { 0 };

        for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
        {
            auto start { std::chrono::high_resolution_clock::now() };

            for (int i = 0; i < size; i += stride)
                arr[i].id *= 2;

            auto end { std::chrono::high_resolution_clock::now() };

            samples[strideIdx].push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }
    }

    int strideIdx { 0 };
    for (int stride = 1; stride <= 1024; stride *= 2, ++strideIdx)
    {
        m_Strides3.push_back(static_cast<double>(stride));
        m_Results3.push_back(AverageWithoutOutliers(samples[strideIdx]));
    }
}

double dae::ThrashTheCacheDemo::AverageWithoutOutliers(const std::vector<double>& samples)
{
    if (samples.size() <= 2)
    {
        double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
        return sum / static_cast<double>(samples.size());
    }

    auto [min, max] { std::minmax_element(samples.begin(), samples.end()) };
    auto sum = std::accumulate(samples.begin(), samples.end(), 0.0) - (*min) - (*max);
    return sum / static_cast<double>(samples.size() - 2);
}