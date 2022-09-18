// Copyright Alex Coultas, Mice Men Example Project

#pragma once

#include "CoreTypes.h"
#include "Misc/Crc.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/UnrealString.h"
#include "Serialization/StructuredArchive.h"

/**
* Custom 2D Integer vector
* Existing Integer Vector, FIntVector2D, has very limiting support,
* where FIntVector2D has a functionality needed, so custom vector created merging the two
* keeping what is useful
*/

#if !CPP      //noexport class
/**
 * An integer vector in 2D space.
 */
USTRUCT( noexport, BlueprintType)
struct FIntVector2D
{
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IntVector, SaveGame)
		int32 X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IntVector, SaveGame)
		int32 Y;

};

#endif

struct FIntVector2D
{
	union
	{
		struct
		{
			int32 X, Y;
		};
	};

#pragma region Constructors

public:

	/**
	 * Default constructor (no initialization).
	 */
	FIntVector2D();

	/**
	 * Creates and initializes a new instance with the specified coordinates.
	 *
	 * @param InX The x-coordinate.
	 * @param InY The y-coordinate.
	 * @param InZ The z-coordinate.
	 */
	FIntVector2D(int32 InX, int32 InY);

	/**
	 * Constructor
	 *
	 * @param InValue replicated to all components
	 */
	explicit FIntVector2D(int32 InValue);

	/**
	 * Constructor
	 *
	 * @param InVector int vector converted to int
	 */
	explicit FIntVector2D(FIntVector2 InVector);

	/**
	 * Constructor
	 *
	 * @param EForceInit Force init enum
	 */
	explicit FORCEINLINE FIntVector2D(EForceInit);

#pragma endregion

#pragma region Operator Overloaders

public:
	/**
	 * Compares points for equality.
	 *
	 * @param Other The other int point being compared.
	 * @return true if the points are equal, false otherwise..
	 */
	bool operator==(const FIntVector2D& Other) const;

	/**
	 * Compares points for inequality.
	 *
	 * @param Other The other int point being compared.
	 * @return true if the points are not equal, false otherwise..
	 */
	bool operator!=(const FIntVector2D& Other) const;

	/**
	 * Scales this point.
	 *
	 * @param Scale What to multiply the point by.
	 * @return Reference to this point after multiplication.
	 */
	FIntVector2D& operator*=(int32 Scale);

	/**
	 * Divides this point.
	 *
	 * @param Divisor What to divide the point by.
	 * @return Reference to this point after division.
	 */
	FIntVector2D& operator/=(int32 Divisor);

	/**
	 * Adds to this point.
	 *
	 * @param Other The point to add to this point.
	 * @return Reference to this point after addition.
	 */
	FIntVector2D& operator+=(const FIntVector2D& Other);

	/**
	 * Subtracts from this point.
	 *
	 * @param Other The point to subtract from this point.
	 * @return Reference to this point after subtraction.
	 */
	FIntVector2D& operator-=(const FIntVector2D& Other);

	/**
	 * Assigns another point to this one.
	 *
	 * @param Other The point to assign this point from.
	 * @return Reference to this point after assignment.
	 */
	FIntVector2D& operator=(const FIntVector2D& Other);

	/**
	 * Gets the result of scaling on this point.
	 *
	 * @param Scale What to multiply the point by.
	 * @return A new scaled int point.
	 */
	FIntVector2D operator*(int32 Scale) const;

	/**
	 * Gets the result of division on this point.
	 *
	 * @param Divisor What to divide the point by.
	 * @return A new divided int point.
	 */
	FIntVector2D operator/(int32 Divisor) const;

	/**
	 * Gets the result of addition on this point.
	 *
	 * @param Other The other point to add to this.
	 * @return A new combined int point.
	 */
	FIntVector2D operator+(const FIntVector2D& Other) const;

	/**
	 * Gets the result of subtraction from this point.
	 *
	 * @param Other The other point to subtract from this.
	 * @return A new subtracted int point.
	 */
	FIntVector2D operator-(const FIntVector2D& Other) const;

#pragma endregion

#pragma region Helper Functions

public:
	/**
	 * Is vector equal to zero.
	 * @return is zero
	*/
	bool IsZero() const;

	/**
	 * Gets the maximum value in the point.
	 *
	 * @return The maximum value in the point.
	 */
	int32 GetMax() const;

	/**
	 * Gets the minimum value in the point.
	 *
	 * @return The minimum value in the point.
	 */
	int32 GetMin() const;

	/**
	 * Gets the distance of this point from (0,0,0).
	 *
	 * @return The distance of this point from (0,0,0).
	 */
	int32 Size() const;

	/**
	 * Get a textual representation of this vector.
	 *
	 * @return A string describing the vector.
	 */
	FString ToString() const;

	/**
	 * Gets the number of components a point has.
	 *
	 * @return Number of components point has.
	 */

	static int32 Num();

#pragma endregion

//-------------------------------------------------------

#pragma region Base Variables

public:
	/** An int point with zeroed values. */
	static const FIntVector2D ZeroValue;

	/** An int point with INDEX_NONE values. */
	static const FIntVector2D NoneValue;

#pragma endregion

};

FORCEINLINE FIntVector2D::FIntVector2D()
{
}

FORCEINLINE FIntVector2D::FIntVector2D(int32 InX, int32 InY)
	: X(InX)
	, Y(InY)
{
}

FORCEINLINE FIntVector2D::FIntVector2D(int32 InValue)
	: X(InValue)
	, Y(InValue)
{
}

FORCEINLINE FIntVector2D::FIntVector2D(FIntVector2 InVector)
	: X(InVector.X)
	, Y(InVector.Y)
{
}

FORCEINLINE FIntVector2D::FIntVector2D(EForceInit)
	: X(0)
	, Y(0)
{
}

FORCEINLINE bool FIntVector2D::operator==(const FIntVector2D& Other) const
{
	return X == Other.X && Y == Other.Y;
}

FORCEINLINE bool FIntVector2D::operator!=(const FIntVector2D& Other) const
{
	return X != Other.X || Y != Other.Y;
}

FORCEINLINE FIntVector2D& FIntVector2D::operator*=(int32 Scale)
{
	X *= Scale;
	Y *= Scale;

	return *this;
}

FORCEINLINE FIntVector2D& FIntVector2D::operator/=(int32 Divisor)
{
	X /= Divisor;
	Y /= Divisor;

	return *this;
}

FORCEINLINE FIntVector2D& FIntVector2D::operator+=(const FIntVector2D& Other)
{
	X += Other.X;
	Y += Other.Y;

	return *this;
}

FORCEINLINE FIntVector2D& FIntVector2D::operator-=(const FIntVector2D& Other)
{
	X -= Other.X;
	Y -= Other.Y;

	return *this;
}

FORCEINLINE FIntVector2D& FIntVector2D::operator=(const FIntVector2D& Other)
{
	X = Other.X;
	Y = Other.Y;

	return *this;
}

FORCEINLINE FIntVector2D FIntVector2D::operator*(int32 Scale) const
{
	return FIntVector2D(*this) *= Scale;
}

FORCEINLINE FIntVector2D FIntVector2D::operator/(int32 Divisor) const
{
	return FIntVector2D(*this) /= Divisor;
}

FORCEINLINE FIntVector2D FIntVector2D::operator+(const FIntVector2D& Other) const
{
	return FIntVector2D(*this) += Other;
}

FORCEINLINE FIntVector2D FIntVector2D::operator-(const FIntVector2D& Other) const
{
	return FIntVector2D(*this) -= Other;
}

FORCEINLINE int32 FIntVector2D::GetMax() const
{
	return FMath::Max(X, Y);
}

FORCEINLINE int32 FIntVector2D::GetMin() const
{
	return FMath::Min(X, Y);
}

FORCEINLINE int32 FIntVector2D::Num()
{
	return 2;
}

FORCEINLINE int32 FIntVector2D::Size() const
{
	int64 X64 = (int64)X;
	int64 Y64 = (int64)Y;
	return int32(FMath::Sqrt(float(X64 * X64 + Y64 * Y64)));
}

FORCEINLINE bool FIntVector2D::IsZero() const
{
	return *this == ZeroValue;
}

FORCEINLINE FString FIntVector2D::ToString() const
{
	return FString::Printf(TEXT("X=%d Y=%d"), X, Y);
}

FORCEINLINE uint32 GetTypeHash(const FIntVector2D& Vector)
{
	return FCrc::MemCrc_DEPRECATED(&Vector, sizeof(FIntVector2D));
}