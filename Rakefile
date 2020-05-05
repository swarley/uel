require "bundler/gem_tasks"
require "rspec/core/rake_task"
require "rake/extensiontask"

RSpec::Core::RakeTask.new(:spec)

task :default => [:compile, :spec]

Rake::ExtensionTask.new("uel") do |ext|
  ext.name = "uel"
  ext.lib_dir = "lib/"
  ext.ext_dir = "ext/uel"
  ext.gem_spec = Gem::Specification.load('uel.gemspec')
end