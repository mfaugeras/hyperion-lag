/*---------------------------------------------------------------------------*/
/* "Hydro.cpp"                                                               */
/*                                                                           */
/* Hydrodynamics methods.                                                    */
/*---------------------------------------------------------------------------*/

#include <iostream>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include <yaml-cpp/yaml.h>

#include <vtkGenericCell.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "utils.hpp"
#include "HydroVars.hpp"
#include "Hydro.hpp"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void add_cell_field(vtkSmartPointer<vtkUnstructuredGrid> mesh,
                    const std::vector<double>& field,
                    const std::string& field_name)
{



  // Create a VTK double array, insert values and attach it to the mesh
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO : write code here
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  vtkSmartPointer<vtkDoubleArray> array = vtkDoubleArray::New();
  array->SetName(field_name.c_str());
  for(int i = 0;i < field.size();i++){
    array->InsertNextValue(field[i]);
  }
  mesh->GetPointData()->SetVectors(array);

}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void add_node_field(vtkSmartPointer<vtkUnstructuredGrid> mesh,
                    const std::vector<double>& field,
                    const std::string& field_name)
{
  vtkSmartPointer<vtkDoubleArray> array = vtkDoubleArray::New();
  array->SetName(field_name.c_str());
  for(int i = 0;i < field.size();i++){
    array->InsertNextValue(field[i]);
  }
  mesh->GetPointData()->SetVectors(array);
  // Create a VTK double array, insert values and attach it to the mesh
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO : write code here
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void add_vector_node_field(vtkSmartPointer<vtkUnstructuredGrid> mesh,
                           const std::vector<std::pair<double, double>>& field,
                           const std::string& field_name)
{

vtkSmartPointer<vtkDoubleArray> array = vtkDoubleArray::New();
  for(int i = 0;i < field.size();i++){
    array->InsertNextTuple2(field[i].first,field[i].second);
    //array->InsertNextValue();
  }
  mesh->GetPointData()->AddArray(array);


  // Create a VTK double array, insert values and attach it to the mesh
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO : write code here
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

Hydro::Hydro(YAML::Node dataset,
             vtkSmartPointer<vtkUnstructuredGrid> mesh,
             HydroVars *vars)
  : m_dataset(dataset), m_mesh(mesh), m_vars(vars)
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::init()
{
  // Initialize CQs
  m_vars->m_cqs.resize(m_vars->m_nb_cells);

  // Initialize timestep
  m_dt = m_dataset["TimeManagement"]["TimeStep"].as<double>();
  m_dt_staggered = m_dt / 2.0;

  // Load initial node coordinates
  for (int n = 0; n < m_vars->m_nb_nodes; ++n) {
    double coord[3];

    /*coord[0] = m_dataset[0 + n * 3];
    coord[1] = m_dataset[1 + n * 3];
    coord[2] = m_dataset[2 + n * 3];

    m_vars->m_node_coord[n].first = coord[0].first;
*/
    m_mesh->GetPoint(n,coord);
    std::pair<double,double> Pair;
    Pair = {coord[0],coord[1]};
    m_vars->m_node_coord.push_back(Pair);


    // Get node n coordinates and save them to m_vars->m_node_coord
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO : write code here
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }

  // Initialize cell volume
  this->compute_volume();

  // Initialize cell and node mass
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    m_vars->m_cell_mass[c] = m_vars->m_density[c] * m_vars->m_cell_volume[c];
    double node_mass_contrib = 0.25 * m_vars->m_cell_mass[c];

    auto cellule = this->m_mesh->GetCell(c);
    // Get cell c to retrieve its node ids
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO : write code here
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int nb_nodes_for_cell = cellule->GetNumberOfPoints(); // Change this line to get the correct number of nodes
    for (int n = 0; n < nb_nodes_for_cell; ++n) {
      auto node = cellule->GetPointIds()->GetId(n); // Change this line to get the global node id
      m_vars->m_node_mass[node] += node_mass_contrib;
    }
  }

  // Initialize internal energy and sound speed
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    double pressure = m_vars->m_pressure[c];
    double adiabatic_cst = m_vars->m_adiabatic_cst[c];
    double density = m_vars->m_density[c];
    m_vars->m_internal_energy[c] = pressure / ((adiabatic_cst - 1.) * density);
    m_vars->m_sound_speed[c] = std::sqrt(adiabatic_cst * pressure / density);
  }

  std::cout << "[Hydro::init] Initialized hydro\n";
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_volume()
{
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    // Local copy of the vertex coordinates of a cell
    std::pair<double, double> coord[4];

    // Cache local coordinates;
    auto cellule = this->m_mesh->GetCell(c);
    // Get cell c to retrieve its nodes
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO : write code here
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int nb_nodes_of_cell = cellule->GetNumberOfPoints();// Change this line to get the correct number of nodes
    for (int n = 0; n < nb_nodes_of_cell; ++n) {
      double p[3];
      // Get node n coordinates
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // TODO : write code here
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      m_mesh->GetPoint(cellule->GetPointIds()->GetId(n),p);
      coord[n] = std::make_pair(p[0], p[1]);
    }

    this->compute_cqs(coord, c, nb_nodes_of_cell);

    // Compute cell volume using CQs
    double volume = 0.0;
    for (int n = 0; n < nb_nodes_of_cell; ++n) {
      volume += inner_product_2D(coord[n], m_vars->m_cqs[c][n]);
    }
    volume /= 2.0;

    m_vars->m_old_cell_volume[c] = m_vars->m_cell_volume[c];
    m_vars->m_cell_volume[c] = volume;
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_cqs(std::pair<double, double>* coord,
                        int cell_idx, int nb_nodes_of_cell)
{
  // Compute cell corner normals (CQs)
  std::vector<std::pair<double, double>> cell_cqs;
  for (int n = 0; n < nb_nodes_of_cell; ++n) {
    int prev_node_i = (n - 1 + nb_nodes_of_cell) % nb_nodes_of_cell;
    int next_node_i = (n + 1 + nb_nodes_of_cell) % nb_nodes_of_cell;

    auto length = std::make_pair(
      0.5 * (coord[prev_node_i].first - coord[next_node_i].first),
      0.5 * (coord[prev_node_i].second - coord[next_node_i].second));
    cell_cqs.push_back(std::make_pair(-length.second, length.first));
  }
  m_vars->m_cqs[cell_idx] = cell_cqs;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_pressure_force()
{
  // Reset force
  for (int n = 0; n < m_vars->m_nb_nodes; ++n) {
    m_vars->m_force[n] = std::make_pair(0.0, 0.0);
  }

  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    // Get cell c to retrieve its node ids
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO : write code here
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    auto cellule = this->m_mesh->GetCell(c);
    int nb_nodes_for_cell = cellule->GetNumberOfPoints();// Change this line to get the correct number of nodes
    for (int n = 0; n < nb_nodes_for_cell; ++n) {
      auto node = cellule->GetPointIds()->GetId(n); // Change this line to get the global node id
      double force = m_vars->m_pressure[c] +
                     m_vars->m_artificial_viscosity[c] * 20.0;
      m_vars->m_force[node].first += force * m_vars->m_cqs[c][n].first;
      m_vars->m_force[node].second += force * m_vars->m_cqs[c][n].second;
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_artificial_viscosity()
{
  auto coefs = m_dataset["ArtificialViscosity"]["Coefficients"];
  double quadratic_viscosity_coef = coefs["Quadratic"].as<double>();
  double linear_viscosity_coef = coefs["Linear"].as<double>();

  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    double massic_volume_t = 1. / m_vars->m_old_density[c];
    double massic_volume_tpdt = 1. / m_vars->m_density[c];
    double massic_volume_staggered = (massic_volume_t + massic_volume_tpdt) / 2.;
    double derived_massic_volume = (massic_volume_tpdt - massic_volume_t) / m_dt;
    double div_u = derived_massic_volume / massic_volume_staggered;
    if (div_u < 0.) {
      m_vars->m_artificial_viscosity[c] = 1. / massic_volume_staggered *
        (quadratic_viscosity_coef * std::pow(m_vars->m_cell_volume[c], 2.0) * std::pow(div_u, 2.0) +
        linear_viscosity_coef * m_vars->m_cell_volume[c] * m_vars->m_sound_speed[c] * std::abs(div_u));
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_velocity()
{
  for (int n = 0; n < m_vars->m_nb_nodes; ++n) {
    auto old_velocity = m_vars->m_velocity[n];
    m_vars->m_velocity[n].first = old_velocity.first +
      (m_dt_staggered / m_vars->m_node_mass[n]) * m_vars->m_force[n].first;
    m_vars->m_velocity[n].second = old_velocity.second +
      (m_dt_staggered / m_vars->m_node_mass[n]) * m_vars->m_force[n].second;
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::apply_boundary_condition(const std::map<int, std::vector<std::string>>& node_envs)
{
  // Y velocity is null on all boundaries
  // X velocity is null on left and right boundaries

  std::map<std::string, YAML::Node> bcs;

  auto bcs_node = m_dataset["BoundaryConditions"];
  for (YAML::const_iterator it = bcs_node.begin(); it != bcs_node.end(); ++it) {
    const YAML::Node& bc = *it;
    bcs[bc["Position"].as<std::string>()] = bc;
  }

  for (const auto& [msh_node_i, boundaries] : node_envs) {
    int n = msh_node_i - 1;

    for (const auto& bc_name : boundaries) {
      const YAML::Node& bc = bcs[bc_name];
      if (auto x = bc["Value"]["X"]) {
        m_vars->m_velocity[n].first = x.as<double>();
      }
      if (auto y = bc["Value"]["Y"]) {
        m_vars->m_velocity[n].second = y.as<double>();
      }
    }
  }

}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::move_nodes()
{
  for (int n = 0; n < m_vars->m_nb_nodes; ++n) {
    m_vars->m_node_coord[n].first += m_dt * m_vars->m_velocity[n].first;
    m_vars->m_node_coord[n].second += m_dt * m_vars->m_velocity[n].second;

    double coord[2] = {m_vars->m_node_coord[n].first,
                      m_vars->m_node_coord[n].second};
    this->m_mesh->GetPoints()->SetPoint(n,coord);
    // Update m_mesh node position
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // TODO : write code here
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::update_density()
{
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    double density = m_vars->m_density[c];
    m_vars->m_old_density[c] = density;
    double new_density = m_vars->m_cell_mass[c] / m_vars->m_cell_volume[c];
    m_vars->m_density[c] = new_density;
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::apply_eos()
{
  // Compute internal energy
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    double adiabatic_cst = m_vars->m_adiabatic_cst[c];
    double volume_ratio = m_vars->m_cell_volume[c] / m_vars->m_old_cell_volume[c];
    double x = 0.5 * (adiabatic_cst - 1.);
    double numer_accrois_nrj = 1. + x * (1. - volume_ratio);
    double denom_accrois_nrj = 1. + x * (1. - 1. / volume_ratio);
    m_vars->m_internal_energy[c] *= numer_accrois_nrj / denom_accrois_nrj;
  }

  // Compute pressure and sound speed
  for (int c = 0; c < m_vars->m_nb_cells; ++c) {
    double internal_energy = m_vars->m_internal_energy[c];
    double density = m_vars->m_density[c];
    double adiabatic_cst = m_vars->m_adiabatic_cst[c];
    double pressure = (adiabatic_cst - 1.0) * density * internal_energy;
    m_vars->m_pressure[c] = pressure;
    double sound_speed = std::sqrt(adiabatic_cst * pressure / density);
    m_vars->m_sound_speed[c] = sound_speed;
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::compute_dt()
{
  m_dt_staggered = m_dt;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Hydro::dump(int step, double simulation_time)
{
  std::cout << "[Hydro::dump] Iteration " << step << " -- Time : "
    << simulation_time << " s -- Time step : " << m_dt << " s\n";

  // Attach the simulation time to the mesh
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO : write code here
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  add_cell_field(m_mesh, m_vars->m_pressure, "Pressure");
  add_cell_field(m_mesh, m_vars->m_artificial_viscosity, "ArtificialViscosity");
  add_cell_field(m_mesh, m_vars->m_internal_energy, "InternalEnergy");
  add_cell_field(m_mesh, m_vars->m_density, "Density");
  add_cell_field(m_mesh, m_vars->m_cell_mass, "Mass");
  add_cell_field(m_mesh, m_vars->m_cell_volume, "Volume");
  add_cell_field(m_mesh, m_vars->m_sound_speed, "SoundSpeed");
  add_node_field(m_mesh, m_vars->m_node_mass, "NodeMass");
  add_vector_node_field(m_mesh, m_vars->m_velocity, "NodeVelocity");
  add_vector_node_field(m_mesh, m_vars->m_force, "NodeForce");

  std::string file_name = "HydroLag." + std::to_string(step) + ".vtu";


  m_writer->SetFileName(file_name.c_str());
  m_writer->SetInputData(m_mesh);
  m_writer->Write();
  // Write the solutions to file_name
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // TODO : write code here
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
