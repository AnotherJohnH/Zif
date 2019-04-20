
#include <cstdint>
#include <cstdlib>
#include <typeinfo>
#include <array>

//! Data for a struct data member
class Member
{
public:
   using Flags = uint32_t;
   
   static const Flags HEX = 1<<0;
   
   enum class Type : uint8_t
   {
      PAD,
      BOOL,
      ENUM,
      SIGNED,
      UNSIGNED,
      FLOAT
   };
   
   Member() = default;

   void decl(const char* name_,
             Type        type_,
             size_t      size_,
             size_t      elements_,
             uint32_t    flags_)
   {
      name     = name_;
      elements = uint32_t(elements_);
      size     = uint8_t(size_);
      type     = type_;
      flags    = flags_;
   }
   
   //! Write data member to a file stream
   void write(FILE* fp, const void* that) const
   {
      switch(type)
      {
      case Type::PAD: break;
      case Type::BOOL: break;
      case Type::ENUM: break;
      case Type::SIGNED: break;
      case Type::UNSIGNED: break;
      case Type::FLOAT: break;
      }
   }
   
   //! Read a data member value from a file stream

private:       
   const char* name{nullptr};   //!< Name
   uint32_t    elements{0};     //!< Number of elements
   uint8_t     size{0};         //!< Data size (bytes)
   Type        type{Type::PAD}; //!< Type
   uint8_t     flags{0};
};


//! Object serialisation interface
class StructBase
{
public:
   void setName(const char* name_)
   {
      name = name_;
   }

   bool exists() const
   {
      return false;
   }

   bool read(void* that) const
   {
      return false;
   }

   void write(const void* that)
   {
      const uint8_t* raw = (const uint8_t*)(that);
      FILE*          fp  = nullptr;

      for(size_t i=0; i < size(); i++)
      {
         at(i).write(fp, raw);
         raw++;
      }
   }

protected:
   virtual void decl(size_t       offset_,
                     const char*  name_,
                     Member::Type type_,
                     size_t       size_,
                     size_t       elements_,
                     uint32_t     flags_ = 0) = 0;

   virtual size_t size() const = 0;

   virtual       Member& at(size_t) = 0;

   virtual const Member& at(size_t) const = 0;

private:
   const char* name{nullptr};
};


//! Member data for a structure
template <unsigned SIZE>
class Struct : public StructBase
{
public:
   virtual void decl(size_t       offset_,
                     const char*  name_,
                     Member::Type type_,
                     size_t       size_,
                     size_t       elements_,
                     uint32_t     flags_ = 0) override
   {
      member_list[offset_].decl(name_, type_, size_, elements_, flags_);
   }

private:
   std::array<Member,SIZE> member_list;

   virtual size_t size() const override { return member_list.size(); }

   virtual const Member& at(size_t index) const override { return member_list[index]; }

   virtual       Member& at(size_t index) override       { return member_list[index]; }
};


//! Class to help determine array element type without running into SFINAE problems
//  in ClassBase::addMember
template <typename TYPE, bool is_array>
struct Element;

template <typename TYPE>
struct Element<TYPE,false>
{
   using ELEMENT = TYPE;
};

template <typename TYPE>
struct Element<TYPE,true>
{
   TYPE tmp; // TODO avoid this declaration

   using ELEMENT = typename std::remove_reference<decltype(tmp[0])>::type;
};


template <typename TYPE>
class Oil
{
public:
   Oil(const char* name_)
   {
      oil().setName(name_);
   }

   //! An oil file exists
   bool exists() const { oil().exists(); }

   //! Read an oil file
   bool read() { return oil().read(this); }

   //! Write an oil file
   void write() { oil().write(this); }

protected:
   //! Declare data member
   template <typename MEMBER>
   void decl(MEMBER& member, const char* name, size_t elements = 1)
   {
      size_t offset = uintptr_t(&member) - uintptr_t((MEMBER*)this);
#if !defined(NO_RTTI)
      if (typeid(MEMBER) == typeid(bool))
      {
         oil().decl(offset, name, Member::Type::BOOL, sizeof(MEMBER), elements);
      }
      else if (std::is_floating_point<MEMBER>::value)
      {
         oil().decl(offset, name, Member::Type::FLOAT, sizeof(MEMBER), elements);
      }
      else if (std::is_enum<MEMBER>::value)
      {
         oil().decl(offset, name, Member::Type::ENUM, sizeof(MEMBER), elements);
      }
      else if (std::is_integral<MEMBER>::value)
      {
         if (std::is_signed<MEMBER>::value)
         {
             oil().decl(offset, name, Member::Type::SIGNED, sizeof(MEMBER), elements);
         }
         else
         {
             oil().decl(offset, name, Member::Type::UNSIGNED, sizeof(MEMBER), elements);
         }
      }
      else if (std::is_array<MEMBER>::value)
      {
         using ELEMENT = typename Element<MEMBER,std::is_array<MEMBER>::value>::ELEMENT;

         decl<ELEMENT>(member, name, sizeof(MEMBER) / sizeof(ELEMENT));
      }
#endif
   }

private:
   StructBase& oil()
   {
      static Struct<sizeof(TYPE)> structure;
      return structure;
   }
};


struct Config : public Oil<Config>
{
   Config() : Oil<Config>("Config")
   {
      decl(font_size,     "font_size");
      decl(border_pixels, "border_pixels");
      decl(line_space,    "line_space");
      decl(invert_video,  "invert_video");
      decl(sleep,         "sleep");
      decl(bg_colour,     "bg_colour");
      decl(fg_colour,     "fg_colour");
   }

   unsigned font_size{18};
   unsigned border_pixels{0};
   unsigned line_space{0};
   bool     invert_video{false};
   unsigned sleep{0};
#ifdef PROJ_TARGET_Kindle3
   uint32_t bg_colour{0xFFFFFF};
   uint32_t fg_colour{0x000000};
#else
   uint32_t bg_colour{0xF0F0E0};
   uint32_t fg_colour{0x382800};
#endif
};


int main()
{
   Config config;

   config.write();
}

