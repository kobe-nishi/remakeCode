#pragma once

#include <H5Cpp.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#define FMT_USE_CONSTEVAL 0
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace xdmf3 {

namespace fs = std::filesystem;

enum class Format { ASCII, Binary, HDF, Unknown };

enum class GeometryType { XYZ, ORIGIN_DXDY, ORIGIN_DXDYDZ, Unknown };

enum class TopologyType : size_t {
  Polyvertex    = 1,
  Polyline      = 2,
  Triangle      = 3,
  Quadrilateral = 4,
  Tetrahedron   = 4,
  Hexahedron    = 8,
  Wedge         = 6,
  Pyramid       = 5,
  CoRectMesh2D  = (size_t)-3,
  CoRectMesh3D  = (size_t)-2,
  Unknown       = (size_t)-1
};

enum class AttributeType : size_t {
  Scalar  = 1,
  Vector  = 3,
  Tensor6 = 6,
  Tensor  = 9,
  Matrix,
  Unknown,
};

enum class AttributeCenter { Node, Cell, Grid, Edge, Face, Unknown };

template <typename T>
inline T from_string(const std::string& str);

inline std::string to_string(Format format) {
  if (format == Format::ASCII)
    return "XML";
  if (format == Format::Binary)
    return "Binary";
  if (format == Format::HDF)
    return "HDF";
  return "Unknown";
}

template <>
inline Format from_string<Format>(const std::string& str) {
  if (str == "XML")
    return Format::ASCII;
  if (str == "Binary")
    return Format::Binary;
  if (str == "HDF")
    return Format::HDF;
  return Format::Unknown;
}

inline std::string to_string(GeometryType type) {
  if (type == GeometryType::XYZ)
    return "XYZ";
  if (type == GeometryType::ORIGIN_DXDY)
    return "ORIGIN_DXDY";
  if (type == GeometryType::ORIGIN_DXDYDZ)
    return "ORIGIN_DXDYDZ";
  return "Unknown";
}

template <>
inline GeometryType from_string<GeometryType>(const std::string& str) {
  if (str == "XYZ")
    return GeometryType::XYZ;
  if (str == "ORIGIN_DXDY")
    return GeometryType::ORIGIN_DXDY;
  if (str == "ORIGIN_DXDYDZ")
    return GeometryType::ORIGIN_DXDYDZ;
  return GeometryType::Unknown;
}

inline std::string to_string(TopologyType type) {
  if (type == TopologyType::Polyvertex)
    return "Polyvertex";
  if (type == TopologyType::Polyline)
    return "Polyline";
  if (type == TopologyType::Triangle)
    return "Triangle";
  if (type == TopologyType::Quadrilateral)
    return "Quadrilateral";
  if (type == TopologyType::Tetrahedron)
    return "Tetrahedron";
  if (type == TopologyType::Hexahedron)
    return "Hexahedron";
  if (type == TopologyType::Wedge)
    return "Wedge";
  if (type == TopologyType::Pyramid)
    return "Pyramid";
  if (type == TopologyType::CoRectMesh2D)
    return "2DCoRectMesh";
  if (type == TopologyType::CoRectMesh3D)
    return "3DCoRectMesh";
  return "Unknown";
}

template <>
inline TopologyType from_string<TopologyType>(const std::string& str) {
  if (str == "Polyvertex")
    return TopologyType::Polyvertex;
  if (str == "Polyline")
    return TopologyType::Polyline;
  if (str == "Triangle")
    return TopologyType::Triangle;
  if (str == "Quadrilateral")
    return TopologyType::Quadrilateral;
  if (str == "Tetrahedron")
    return TopologyType::Tetrahedron;
  if (str == "Hexahedron")
    return TopologyType::Hexahedron;
  if (str == "Wedge")
    return TopologyType::Wedge;
  if (str == "Pyramid")
    return TopologyType::Pyramid;
  if (str == "2DCoRectMesh")
    return TopologyType::CoRectMesh2D;
  if (str == "3DCoRectMesh")
    return TopologyType::CoRectMesh3D;
  return TopologyType::Unknown;
}

inline std::string to_string(AttributeType type) {
  if (type == AttributeType::Scalar)
    return "Scalar";
  if (type == AttributeType::Vector)
    return "Vector";
  if (type == AttributeType::Tensor6)
    return "Tensor6";
  if (type == AttributeType::Tensor)
    return "Tensor";
  return "Matrix";
}

template <>
inline AttributeType from_string<AttributeType>(const std::string& str) {
  if (str == "Scalar")
    return AttributeType::Scalar;
  if (str == "Vector")
    return AttributeType::Vector;
  if (str == "Tensor")
    return AttributeType::Tensor;
  if (str == "Tensor6")
    return AttributeType::Tensor6;
  if (str == "Matrix")
    return AttributeType::Matrix;
  return AttributeType::Unknown;
}

inline std::string to_string(AttributeCenter type) {
  if (type == AttributeCenter::Node)
    return "Node";
  if (type == AttributeCenter::Cell)
    return "Cell";
  if (type == AttributeCenter::Grid)
    return "Grid";
  if (type == AttributeCenter::Edge)
    return "Edge";
  if (type == AttributeCenter::Face)
    return "Face";
  return "Unknown";
}

template <>
inline AttributeCenter from_string<AttributeCenter>(const std::string& str) {
  if (str == "Node")
    return AttributeCenter::Node;
  if (str == "Cell")
    return AttributeCenter::Cell;
  if (str == "Grid")
    return AttributeCenter::Grid;
  if (str == "Edge")
    return AttributeCenter::Edge;
  if (str == "Face")
    return AttributeCenter::Face;
  return AttributeCenter::Unknown;
}

struct DataItem {
  Format              format;
  std::string         dataType  = "Float";
  int                 precision = 8;
  std::vector<size_t> dimensions;
  std::string         heavyDataPath;
  std::string         root_path = "./";

  template <typename T>
  const DataItem& read(T** out) const {
    auto errorWrite = [&] (const std::string &type) -> void {
      throw std::runtime_error("Data type does not match. dataType = " + type + ", T = " +
                               std::string(typeid(T).name()));
    };
    if (dataType == "Float") {
      if (precision == 4) {
        if (!std::is_same<T, float>::value) {
          errorWrite("float"); 
        }
      } else if (precision == 8) {
        if (!std::is_same<T, double>::value) {
          errorWrite("double");
        }
      }
    } else if (dataType == "Int") {
      if (precision == 4) {
        if (!std::is_same<T, int>::value && !std::is_same<T, int32_t>::value) {
          errorWrite("int");
        }
      } else if (precision == 8) {
        if (!std::is_same<T, long long>::value && !std::is_same<T, int64_t>::value) {
          errorWrite("long long");
        }
      }
    } else if (dataType == "UInt") {
      if (precision == 4) {
        if (!std::is_same<T, unsigned int>::value && !std::is_same<T, uint32_t>::value) {
          errorWrite("unsigned int");
        }
      } else if (precision == 8) {
        if (!std::is_same<T, unsigned long long>::value && !std::is_same<T, uint64_t>::value) {
          errorWrite("unsigned long long");
        }
      }
    }

    size_t totalElements = 1;
    for (auto d : dimensions) totalElements *= d;
    if (totalElements < 1) {
      *out = nullptr;
      return *this;
    }
    *out = new T[totalElements];
    if (format == Format::ASCII) {
      std::istringstream iss(heavyDataPath);
      size_t             idx = 0;
      while (iss && idx < totalElements) {
        int tmp;
        iss >> (*out)[idx++];
      }
    } else if (format == Format::Binary) {
      std::ifstream ifs(root_path + heavyDataPath, std::ios::binary);
      if (!ifs.is_open())
        std::runtime_error("File cannot open. Filename = " + root_path + heavyDataPath);
      ifs.read(reinterpret_cast<char*>(*out), sizeof(T) * totalElements);
    } else if (format == Format::HDF) {
      auto pos = heavyDataPath.find(":/");
      if (pos == std::string::npos)
        std::runtime_error("File path is wrond. File path = " + heavyDataPath);
      std::string   filename = root_path + heavyDataPath.substr(0, pos);
      std::string   dataname = heavyDataPath.substr(pos + 1);  // keep leading '/'
      H5::H5File    file(filename, H5F_ACC_RDONLY);
      H5::DataSet   dataset   = file.openDataSet(dataname);
      H5::DataSpace dataspace = dataset.getSpace();
      hsize_t*      dims_h5   = new hsize_t[dataspace.getSimpleExtentNdims()];
      dataspace.getSimpleExtentDims(dims_h5);
      dataset.read(*out, dataset.getDataType());
      delete[] dims_h5;
      dataset.close();
      file.close();
    }
    return *this;
  }

  std::string to_xml(int indent = 0) const {
    std::string ind(indent, ' ');
    std::string dimStr;
    for (size_t i = 0; i < dimensions.size(); ++i) {
      dimStr += std::to_string(dimensions[i]) + (i == dimensions.size() - 1 ? "" : " ");
    }

    std::string formatStr = to_string(format);
    std::string dataContent;
    if (format == Format::ASCII) {
      std::string        dataInd = ind + "  ";
      std::istringstream iss(heavyDataPath);
      std::string        line;
      while (std::getline(iss, line)) {
        // Trim leading/trailing whitespace from the line before re-indenting
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
          continue;
        size_t      last    = line.find_last_not_of(" \t\r\n");
        std::string trimmed = line.substr(first, (last - first + 1));
        dataContent += dataInd + trimmed + "\n";
      }
    } else {
      dataContent = ind + "  " + heavyDataPath + "\n";
    }
    return ind + "<DataItem Format=\"" + formatStr + "\" DataType=\"" + dataType + "\" " +
           "Precision=\"" + std::to_string(precision) + "\" Dimensions=\"" + dimStr + "\">\n" +
           dataContent + ind + "</DataItem>";

    // return fmt::format(
    //     "{}<DataItem Format=\"{}\" DataType=\"{}\" "
    //     "Precision=\"{}\" Dimensions=\"{}\">\n{}{}</DataItem>",
    //     ind,
    //     formatStr,
    //     dataType,
    //     precision,
    //     dimStr,
    //     dataContent,
    //     ind);
  }
  size_t              size() const { return dimensions.at(0); }
  size_t              dim() const { return dimensions.at(1); }
  std::vector<size_t> dims() const { return dimensions; }
};

struct Geometry {
  GeometryType          type;
  DataItem              data;
  std::vector<DataItem> dataItems;

  std::string to_xml(int indent = 0) const {
    std::string ind(indent, ' ');
    //std::string xml = fmt::format("{}<Geometry GeometryType=\"{}\">\n", ind, to_string(type));
    std::string xml = ind + "<Geometry GeometryType=\"" + to_string(type) + "\">\n";
    if (!dataItems.empty()) {
      for (const auto& item : dataItems) {
        xml += item.to_xml(indent + 2) + "\n";
      }
    } else {
      xml += data.to_xml(indent + 2) + "\n";
    }
    // xml += fmt::format("{}</Geometry>", ind);
    xml += ind + "</Geometry>";
    return xml;
  }
};

struct Topology {
  TopologyType        type;
  size_t              numberOfElements;
  std::vector<size_t> dimensions;
  DataItem            data;

  std::string to_xml(int indent = 0) const {
    std::string ind(indent, ' ');
    if (type == TopologyType::CoRectMesh2D || type == TopologyType::CoRectMesh3D ||
        type == TopologyType::Polyvertex) {
      std::string dimStr;
      for (size_t i = 0; i < dimensions.size(); ++i) {
        dimStr += std::to_string(dimensions[i]) + (i == dimensions.size() - 1 ? "" : " ");
      }
    //   return fmt::format(
    //       "{}<Topology TopologyType=\"{}\" Dimensions=\"{}\"/>", ind, to_string(type), dimStr);
    return ind + "<Topology TopologyType=\"" + to_string(type) + "\" Dimensions=\"" + dimStr + "\"/>";
    } else if (type == TopologyType::Polyline) {
        return ind + "<Topology TopologyType=\"" + to_string(type) + "\" " +
                    "NumberOfElements=\"" + std::to_string(numberOfElements) + "\" NodesPerElement=\"" + std::to_string(dimensions.at(1)) + "\">\n" +
                    data.to_xml(indent + 2) + "\n" +
                    ind + "</Topology>";
            }
            return ind + "<Topology TopologyType=\"" + to_string(type) + "\" " +
                "NumberOfElements=\"" + std::to_string(numberOfElements) + "\">\n" +
                data.to_xml(indent + 2) + "\n" +
                ind + "</Topology>";
        }
//       return fmt::format(
//           "{}<Topology TopologyType=\"{}\" "
//           "NumberOfElements=\"{}\" NodesPerElement=\"{}\">\n{}\n{}</Topology>",
//           ind,
//           to_string(type),
//           numberOfElements,
//           dimensions.at(1),
//           data.to_xml(indent + 2),
//           ind);
//     }
//     return fmt::format(
//         "{}<Topology TopologyType=\"{}\" "
//         "NumberOfElements=\"{}\">\n{}\n{}</Topology>",
//         ind,
//         to_string(type),
//         numberOfElements,
//         data.to_xml(indent + 2),
//         ind);
//   }
};

struct Attribute {
  std::string     name;
  AttributeCenter center;
  AttributeType   type;
  DataItem        data;

  std::string to_xml(int indent = 0) const {
    std::string ind(indent, ' ');
//     return fmt::format(
//         "{}<Attribute Name=\"{}\" AttributeType=\"{}\" "
//         "Center=\"{}\">\n{}\n{}</Attribute>",
//         ind,
//         name,
//         to_string(type),
//         to_string(center),
//         data.to_xml(indent + 2),
//         ind);
//   }
return ind + "<Attribute Name=\"" + name + "\" AttributeType=\"" + to_string(type) + "\" " +
           "Center=\"" + to_string(center) + "\">\n" +
           data.to_xml(indent + 2) + "\n" +
           ind + "</Attribute>";
  }
};

struct Grid {
  std::string            name;
  std::optional<double>  time;
  std::optional<int>     step;
  Geometry               geometry;
  Topology               topology;
  std::vector<Attribute> attributes;
  Grid() = default;
  Grid(const std::string& name_, const double& time_) : name(name_), time(time_) {}
  Grid(const std::string& name_, const double& time_, const int& step_)
      : name(name_), time(time_), step(step_) {}

  std::string to_xml(int indent = 0) const {
    std::string ind(indent, ' ');
    // std::string xml = fmt::format("{}<Grid Name=\"{}\" GridType=\"Uniform\">\n", ind, name);
    // if (time.has_value()) {
    //   xml += fmt::format("{}  <Time Value=\"{:.5e}\" />\n", ind, *time);
    // }
    std::string xml = ind + "<Grid Name=\"" + name + "\" GridType=\"Uniform\">\n";
        if (time.has_value()) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.5e", *time);
        xml += ind + "  <Time Value=\"" + std::string(buf) + "\" />\n";
        }
    xml += geometry.to_xml(indent + 2) + "\n";
    xml += topology.to_xml(indent + 2) + "\n";
    for (const auto& attr : attributes) {
      xml += attr.to_xml(indent + 2) + "\n";
    }
    // xml += fmt::format("{}</Grid>", ind);
    xml += ind + "</Grid>";
    return xml;
  }
  double get_time() const { return time.value(); }

  template <typename T>
  const DataItem& read(T** out, const std::string& name) const {
    if (name == "Geometry") {
      if (geometry.dataItems.size() == 2) {
        int dim = geometry.dataItems.at(0).dim();
        *out    = new T[2 * dim];
        T* tmp1 = nullptr;
        T* tmp2 = nullptr;

        geometry.dataItems.at(0).read(&tmp1);
        geometry.dataItems.at(1).read(&tmp2);

        for (int i = 0; i < dim; i++) {
          int j           = dim - i - 1;
          (*out)[i]       = tmp1[j];
          (*out)[i + dim] = tmp2[j];
        }
        delete[] tmp1;
        delete[] tmp2;
        return geometry.dataItems.at(0);
      } else {
        return geometry.data.read(out);
      }
    } else if (name == "Topology") {
      if (topology.type == TopologyType::CoRectMesh2D ||
          topology.type == TopologyType::CoRectMesh3D ||
          topology.type == TopologyType::Polyvertex) {
        return topology.data;
      } else {
        return topology.data.read(out);
      }
    } else {
      auto target = std::find_if(
          attributes.begin(), attributes.end(), [&name](auto& attr) { return attr.name == name; });
      if (target == attributes.end())
        throw std::runtime_error("No matched attribute is found. Attribute name = " + name);
      return target->data.read(out);
    }
  }
};

class Writer {
 public:
  Writer(const std::string& filename, Format format, std::vector<Grid> grids = {})
      : m_filename(filename), m_format(format), m_grids(grids) {
    m_baseDir         = fs::path(filename).parent_path();
    m_baseName        = fs::path(filename).stem().string();
    fs::path rootPath = fs::path(m_filename).parent_path();
    if (!fs::exists(rootPath))
      fs::create_directories(rootPath);
  }

  void add_grid(const Grid& grid) {
    auto it = std::find_if(
        m_grids.begin(), m_grids.end(), [&grid](const Grid& g) { return g.name == grid.name; });
    if (it != m_grids.end()) {
      *it = grid;
    } else {
      m_grids.push_back(grid);
    }
  }

  void write(bool append = false) {
    bool isTimeSeries = !m_grids.empty() && m_grids[0].time.has_value();
    bool fileExists   = fs::exists(m_filename);
    bool shouldAppend = append && fileExists;

    if (shouldAppend) {
      std::ifstream ifs(m_filename, std::ios::binary | std::ios::ate);
      if (ifs) {
        long long fileSize = ifs.tellg();
        long long readSize = std::min(fileSize, 10LL * 1024LL * 1024LL);  // 10MB
        ifs.seekg(-readSize, std::ios::end);
        std::string buffer(readSize, ' ');
        ifs.read(&buffer[0], readSize);
        ifs.close();

        long long firstConflictPos = -1;
        for (const auto& grid : m_grids) {
        //   std::string searchStr = fmt::format("<Grid Name=\"{}\"", grid.name);
        std::string searchStr = "<Grid Name=\"" + grid.name + "\"";
          size_t      pos       = buffer.find(searchStr);
          if (pos != std::string::npos) {
            // Walk back to the start of the line to include leading whitespace
            size_t lineStart = pos;
            while (lineStart > 0 && buffer[lineStart - 1] != '\n') {
              --lineStart;
            }
            long long absPos = fileSize - readSize + lineStart;
            if (firstConflictPos == -1 || absPos < firstConflictPos) {
              firstConflictPos = absPos;
            }
          }
        }

        if (firstConflictPos != -1) {
          fs::resize_file(m_filename, firstConflictPos);
        } else {
          size_t domainPos = buffer.rfind("  </Domain>");
          if (domainPos != std::string::npos) {
            size_t    gridPos  = buffer.rfind("    </Grid>", domainPos);
            long long truncPos = -1;
            if (gridPos != std::string::npos && isTimeSeries) {
              truncPos = fileSize - readSize + gridPos;
            } else {
              truncPos = fileSize - readSize + domainPos;
            }

            if (truncPos != -1) {
              fs::resize_file(m_filename, truncPos);
            } else {
              shouldAppend = false;
            }
          } else {
            shouldAppend = false;
          }
        }
      } else {
        shouldAppend = false;
      }
    }

    std::ofstream xdmf(m_filename, shouldAppend ? std::ios::app : std::ios::trunc);
    if (!xdmf.is_open())
      return;

    if (!shouldAppend) {
      xdmf << "<?xml version=\"1.0\" ?>\n";
      xdmf << "<!DOCTYPE Xdmf SYSTEM \"Xdmf.dtd\" []>\n";
      xdmf << "<Xdmf Version=\"3.0\">\n";
      xdmf << "  <Domain>\n";
      if (isTimeSeries) {
        xdmf << "    <Grid Name=\"TimeSeriesData\" GridType=\"Collection\" "
                "CollectionType=\"Temporal\">\n";
      }
    }

    if (isTimeSeries) {
      for (const auto& grid : m_grids) {
        xdmf << grid.to_xml(6) << "\n";
      }
      xdmf << "    </Grid>\n";
    } else {
      for (const auto& grid : m_grids) {
        xdmf << grid.to_xml(4) << "\n";
      }
    }

    xdmf << "  </Domain>\n";
    xdmf << "</Xdmf>\n";
    clear_grids();
  }

  template <typename T>
  std::string write_data(const std::string&         rootName,
                         const std::string&         attrName,
                         const T*                   data,
                         const std::vector<size_t>& dims,
                         int                        step = -1) {
    std::string dataType = get_data_type<T>();

    std::string groupName = rootName;
    if (step < 0) {
      if (attrName == "Geometry" || attrName == "Origin" || attrName == "DxDyDz")
        groupName = "geo";
      else if (attrName == "Topology")
        groupName = "ele";
    }

    size_t totalElements = 1;
    for (auto d : dims) totalElements *= d;

    if (m_format == Format::HDF) {
      std::string h5path = m_baseName + ".h5";
      if (step != -2 && totalElements > 0)
        write_hdf5_internal(h5path, groupName, attrName, data, dims);
    //   return fmt::format("{}:/{}/{}", h5path, groupName, attrName);
    return h5path + ":/" + groupName + "/" + attrName;
    } else if (m_format == Format::Binary) {
      std::string dirName = m_baseName + "_bin";
      fs::path    subDir  = m_baseDir / dirName / groupName;
      fs::create_directories(subDir);

      std::string ext      = ".bin";
      std::string filename = attrName + ext;
      fs::path    filePath = subDir / filename;

      if (step > -2 && totalElements > 0) {
        std::ofstream ofs(filePath, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data), totalElements * sizeof(T));
      }
      return dirName + "/" + groupName + "/" + filename;
    } else {
      // Format::ASCII: Use in-line data
      std::ostringstream oss;
      size_t             stride = (dims.size() > 1) ? dims.back() : 0;
      for (size_t i = 0; i < totalElements; ++i) {
        if constexpr (std::is_integral_v<T>) {
          oss << data[i];
        } else {
          if constexpr (sizeof(T) == 4) {
            oss << fmt::format("{:+.7e}", data[i]);
          } else {
            oss << fmt::format("{:+.15e}", data[i]);
          }
        }
        if (i + 1 < totalElements) {
          oss << ((stride > 0 && (i + 1) % stride == 0) ? "\n" : " ");
        }
      }
      return oss.str();
    }
  }

  template <typename T>
  void set_geometry(Grid&               grid,
                    const T*            data,
                    const size_t&       numNodeDim,
                    const GeometryType& geomType           = GeometryType::XYZ,
                    bool                is_change_geometry = true) {
    int step = (is_change_geometry ? grid.step.value() : (m_is_first_geometry ? -1 : -2));
    if (m_is_first_geometry)
      m_is_first_geometry = false;
    grid.geometry.type = geomType;
    grid.geometry.dataItems.clear();

    bool isCoRectGeom =
        (geomType == GeometryType::ORIGIN_DXDY || geomType == GeometryType::ORIGIN_DXDYDZ);
    if (isCoRectGeom) {
      const std::vector<size_t> splitDims = {1, numNodeDim};
      std::vector<T>            origin(data, data + numNodeDim);
      std::vector<T>            spacing(data + numNodeDim, data + 2 * numNodeDim);
      std::reverse(origin.begin(), origin.end());
      std::reverse(spacing.begin(), spacing.end());

      std::string originPath  = write_data(grid.name, "Origin", origin.data(), splitDims, step);
      std::string spacingPath = write_data(grid.name, "DxDyDz", spacing.data(), splitDims, step);

      DataItem originItem{m_format, get_data_type<T>(), (int)sizeof(T), splitDims, originPath};
      DataItem spacingItem{m_format, get_data_type<T>(), (int)sizeof(T), splitDims, spacingPath};

      grid.geometry.data      = originItem;
      grid.geometry.dataItems = {originItem, spacingItem};
      return;
    }

    const std::vector<size_t> dims = {numNodeDim, 3};
    std::string               path = write_data(grid.name, "Geometry", data, dims, step);
    grid.geometry.data             = {m_format, get_data_type<T>(), (int)sizeof(T), dims, path};
  }

  template <typename T>
  void set_topology(Grid&               grid,
                    const T*            data,
                    const size_t&       numberOfElements,
                    const TopologyType& type,
                    bool                is_change_topology = true) {
    const std::vector<size_t> dims = {numberOfElements, static_cast<size_t>(type)};
    int step = (is_change_topology ? grid.step.value() : (m_is_first_topology ? -1 : -2));
    if (m_is_first_topology)
      m_is_first_topology = false;

    std::string path = "";
    if (type != TopologyType::Polyvertex)
      path = write_data(grid.name, "Topology", data, dims, step);

    grid.topology.type             = type;
    grid.topology.numberOfElements = numberOfElements;
    grid.topology.dimensions       = dims;
    grid.topology.data             = {m_format, get_data_type<T>(), (int)sizeof(T), dims, path};
  }

  void set_topology(Grid& grid, const std::vector<size_t>& dims, const TopologyType& type) {
    grid.topology.type       = type;
    grid.topology.dimensions = dims;
    std::reverse(grid.topology.dimensions.begin(), grid.topology.dimensions.end());
    grid.topology.numberOfElements = 0;
  }

  template <typename T>
  void add_attribute(Grid&                  grid,
                     const T*               data,
                     const size_t&          numberOfData,
                     const AttributeType&   type,
                     const AttributeCenter& center,
                     const std::string&     name) {
    const std::vector<size_t> dims = {numberOfData, static_cast<size_t>(type)};
    std::string               path = write_data(grid.name, name, data, dims, grid.step.value());
    Attribute                 attr;
    attr.name   = name;
    attr.center = center;
    attr.type   = type;
    attr.data   = {m_format, get_data_type<T>(), (int)sizeof(T), dims, path};
    grid.attributes.push_back(attr);
  }

  size_t grid_num() const { return m_grids.size(); }

 private:
  void clear_grids() { m_grids.clear(); }

  template <typename T>
  static std::string get_data_type() {
    if constexpr (std::is_integral_v<T>) {
      return std::is_signed<T>::value ? "Int" : "UInt";
    }
    return "Float";
  }

  template <typename T>
  void write_hdf5_internal(const std::string&         h5file,
                           const std::string&         groupName,
                           const std::string&         datasetName,
                           const T*                   data,
                           const std::vector<size_t>& dims) {
    fs::path     fullH5Path = m_baseDir / h5file;
    std::string  fullPath   = groupName + "/" + datasetName;
    H5::H5File*  file;
    H5::Group*   group;
    H5::DataSet* dataset;

    auto get_PredType = []() -> H5::PredType {
      if constexpr (std::is_same_v<T, double>)
        return H5::PredType::NATIVE_DOUBLE;
      else if constexpr (std::is_same_v<T, float>)
        return H5::PredType::NATIVE_FLOAT;
      else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>)
        return H5::PredType::NATIVE_INT;
      else if constexpr (std::is_same_v<T, unsigned int> || std::is_same_v<T, uint32_t>)
        return H5::PredType::NATIVE_UINT;
      else if constexpr (std::is_same_v<T, long long> || std::is_same_v<T, int64_t>)
        return H5::PredType::NATIVE_LLONG;
      else if constexpr (std::is_same_v<T, unsigned long long> || std::is_same_v<T, uint64_t>)
        return H5::PredType::NATIVE_ULLONG;
      else if constexpr (sizeof(T) == 1)
        return H5::PredType::NATIVE_CHAR;
    };

    if (fs::exists(fullH5Path)) {
      file = new H5::H5File(fullH5Path.string(), H5F_ACC_RDWR);
    } else {
      file = new H5::H5File(fullH5Path.string(), H5F_ACC_TRUNC);
    }
    if (file->nameExists(groupName)) {
      group = new H5::Group(file->openGroup(groupName));
    } else {
      group = new H5::Group(file->createGroup(groupName));
    }
    hsize_t hdims[2] = {static_cast<hsize_t>(dims[0]), static_cast<hsize_t>(dims[1])};
    hsize_t cdims[2] = {static_cast<hsize_t>(1000 > dims[0] ? dims[0] : 1000),
                        static_cast<hsize_t>(dims[1])};

    H5::DataSpace dataspace(sizeof(hdims) / sizeof(hsize_t), hdims);

    H5::DSetCreatPropList cparms;
    cparms.setChunk(2, cdims);
    cparms.setDeflate(6);
    if (file->nameExists(fullPath)) {
      file->unlink(fullPath);
    }
    dataset = new H5::DataSet(file->createDataSet(fullPath, get_PredType(), dataspace, cparms));
    dataset->write(data, dataset->getDataType());
    delete dataset;
    delete group;
    delete file;
  }

  std::string       m_filename;
  Format            m_format;
  fs::path          m_baseDir;
  std::string       m_baseName;
  std::vector<Grid> m_grids;
  bool              m_is_first_geometry = true;
  bool              m_is_first_topology = true;
};

// Minimal XML Parser for Reading XDMF
class Reader {
 public:
  struct Node {
    std::string                        name;
    std::map<std::string, std::string> attributes;
    std::string                        content;
    std::vector<std::shared_ptr<Node>> children;

    std::string get_attr(const std::string& key) const {
      auto it = attributes.find(key);
      return (it != attributes.end()) ? it->second : "";
    }
  };

  static std::vector<Grid> read(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open())
      return {};
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    std::vector<Grid> grids;

    auto find_tag = [&](const std::string& tag, size_t start) -> size_t {
      return content.find("<" + tag, start);
    };

    auto get_attr = [&](size_t tagPos, const std::string& attr) -> std::string {
      size_t      endPos     = content.find(">", tagPos);
      std::string tagContent = content.substr(tagPos, endPos - tagPos);
      size_t      attrPos    = tagContent.find(attr + "=\"");
      if (attrPos == std::string::npos)
        return "";
      attrPos += attr.length() + 2;
      size_t endAttr = tagContent.find("\"", attrPos);
      return tagContent.substr(attrPos, endAttr - attrPos);
    };

    auto get_content = [&](size_t tagPos, const std::string& tag) -> std::string {
      size_t endOpening = content.find(">", tagPos);
      if (content[endOpening - 1] == '/')
        return "";  // Self-closing
      size_t closeTag = content.find("</" + tag + ">", endOpening);
      if (closeTag == std::string::npos)
        return "";
      return content.substr(endOpening + 1, closeTag - endOpening - 1);
    };

    auto gridPos = find_tag("Grid", 0);
    while (gridPos != std::string::npos) {
      std::string gridType = get_attr(gridPos, "GridType");
      if (gridType == "Collection") {
        gridPos = find_tag("Grid", gridPos + 1);
        continue;
      }

      Grid grid;
      grid.name = get_attr(gridPos, "Name");

      // Parse Time
      size_t timePos = find_tag("Time", gridPos);
      if (timePos != std::string::npos && timePos < find_tag("Grid", gridPos + 1)) {
        std::string timeStr = get_attr(timePos, "Value");
        if (!timeStr.empty()) {
          grid.time = std::stod(timeStr);
        }
      }

      // Parse Geometry
      size_t geomPos = find_tag("Geometry", gridPos);
      if (geomPos != std::string::npos) {
        grid.geometry.type = from_string<GeometryType>(get_attr(geomPos, "GeometryType"));
        size_t geomOpenEnd = content.find(">", geomPos);
        if (geomOpenEnd != std::string::npos) {
          size_t geomClosePos = content.find("</Geometry>", geomOpenEnd);
          size_t searchPos    = geomOpenEnd;
          while (geomClosePos != std::string::npos) {
            size_t diPos = find_tag("DataItem", searchPos);
            if (diPos == std::string::npos || diPos >= geomClosePos)
              break;

            DataItem item;
            item.format         = from_string<Format>(get_attr(diPos, "Format"));
            item.dataType       = get_attr(diPos, "DataType");
            std::string precStr = get_attr(diPos, "Precision");
            if (!precStr.empty())
              item.precision = std::stoi(precStr);

            std::string        dimStr = get_attr(diPos, "Dimensions");
            std::istringstream dss(dimStr);
            size_t             dim;
            while (dss >> dim) item.dimensions.push_back(dim);

            item.heavyDataPath = get_content(diPos, "DataItem");
            if (!item.heavyDataPath.empty()) {
              item.heavyDataPath.erase(0, item.heavyDataPath.find_first_not_of(" \n\r\t"));
              item.heavyDataPath.erase(item.heavyDataPath.find_last_not_of(" \n\r\t") + 1);
            }
            item.root_path = fs::path(filename).parent_path().string() + "/";
            if (grid.geometry.dataItems.empty()) {
              grid.geometry.data = item;
            }
            grid.geometry.dataItems.push_back(item);
            searchPos = diPos + 1;
          }
        }
      }

      // Parse Topology
      size_t topoPos = find_tag("Topology", gridPos);
      if (topoPos != std::string::npos) {
        size_t topoOpenEnd = content.find(">", topoPos);
        bool   topoSelfClosing =
            (topoOpenEnd != std::string::npos && content[topoOpenEnd - 1] == '/');
        size_t topoClosePos = std::string::npos;
        if (!topoSelfClosing && topoOpenEnd != std::string::npos) {
          topoClosePos = content.find("</Topology>", topoOpenEnd);
        }

        grid.topology.type = from_string<TopologyType>(get_attr(topoPos, "TopologyType"));

        std::string numElemStr = get_attr(topoPos, "NumberOfElements");
        if (!numElemStr.empty())
          grid.topology.numberOfElements = std::stoull(numElemStr);

        std::string topoDimStr = get_attr(topoPos, "Dimensions");
        if (!topoDimStr.empty()) {
          std::istringstream tss(topoDimStr);
          size_t             dim;
          while (tss >> dim) grid.topology.dimensions.push_back(dim);
        }

        size_t diPos                    = find_tag("DataItem", topoPos);
        bool   isDataItemInsideTopology = (!topoSelfClosing && diPos != std::string::npos &&
                                         topoClosePos != std::string::npos && diPos < topoClosePos);
        if (isDataItemInsideTopology) {
          grid.topology.data.format   = from_string<Format>(get_attr(diPos, "Format"));
          grid.topology.data.dataType = get_attr(diPos, "DataType");
          std::string precStr         = get_attr(diPos, "Precision");
          if (!precStr.empty())
            grid.topology.data.precision = std::stoi(precStr);

          std::string        dimStr = get_attr(diPos, "Dimensions");
          std::istringstream dss(dimStr);
          size_t             dim;
          while (dss >> dim) grid.topology.data.dimensions.push_back(dim);

          grid.topology.data.heavyDataPath = get_content(diPos, "DataItem");
          grid.topology.data.heavyDataPath.erase(
              0, grid.topology.data.heavyDataPath.find_first_not_of(" \n\r\t"));
          grid.topology.data.heavyDataPath.erase(
              grid.topology.data.heavyDataPath.find_last_not_of(" \n\r\t") + 1);
        } else {
          if (grid.topology.type == TopologyType::CoRectMesh2D ||
              grid.topology.type == TopologyType::CoRectMesh3D) {
            std::reverse(grid.topology.dimensions.begin(), grid.topology.dimensions.end());
          }
          grid.topology.data.dimensions = grid.topology.dimensions;
        }
        grid.topology.data.root_path = fs::path(filename).parent_path().string() + "/";
      }

      // Parse Attributes
      size_t attrPos     = find_tag("Attribute", gridPos);
      size_t nextGridPos = find_tag("Grid", gridPos + 1);
      while (attrPos != std::string::npos &&
             (nextGridPos == std::string::npos || attrPos < nextGridPos)) {
        Attribute attr;
        attr.name   = get_attr(attrPos, "Name");
        attr.type   = from_string<AttributeType>(get_attr(attrPos, "AttributeType"));
        attr.center = from_string<AttributeCenter>(get_attr(attrPos, "Center"));

        size_t diPos = find_tag("DataItem", attrPos);
        if (diPos != std::string::npos) {
          attr.data.format    = from_string<Format>(get_attr(diPos, "Format"));
          attr.data.dataType  = get_attr(diPos, "DataType");
          std::string precStr = get_attr(diPos, "Precision");
          if (!precStr.empty())
            attr.data.precision = std::stoi(precStr);

          std::string        dimStr = get_attr(diPos, "Dimensions");
          std::istringstream dss(dimStr);
          size_t             dim;
          while (dss >> dim) attr.data.dimensions.push_back(dim);

          attr.data.heavyDataPath = get_content(diPos, "DataItem");
          attr.data.heavyDataPath.erase(0, attr.data.heavyDataPath.find_first_not_of(" \n\r\t"));
          attr.data.heavyDataPath.erase(attr.data.heavyDataPath.find_last_not_of(" \n\r\t") + 1);
        }
        attr.data.root_path = fs::path(filename).parent_path().string() + "/";
        grid.attributes.push_back(attr);
        attrPos = find_tag("Attribute", attrPos + 1);
      }
      grids.push_back(grid);
      gridPos = find_tag("Grid", gridPos + 1);
    }

    return grids;
  }
};

}  // namespace xdmf3