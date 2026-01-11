'use client'

interface Position {
  marketId: string
  eventName: string
  side: 'buy' | 'sell'
  amount: number
  price: number
  timestamp: Date
  market: number
}

interface PortfolioViewProps {
  positions: Position[]
  balance: number
}

const marketNames: { [key: number]: string } = {
  0: 'Polymarket',
  1: 'Kalshi',
  2: 'PredictIt'
}

export default function PortfolioView({ positions, balance }: PortfolioViewProps) {
  // Group positions by market
  const positionsByMarket = new Map<string, Position[]>()
  positions.forEach(pos => {
    if (!positionsByMarket.has(pos.marketId)) {
      positionsByMarket.set(pos.marketId, [])
    }
    positionsByMarket.get(pos.marketId)!.push(pos)
  })

  return (
    <>
      {/* Portfolio Header */}
      <div className="page-header">
        <div>
          <h1 className="page-title">Portfolio</h1>
          <p className="page-subtitle">ACTIVE POSITIONS // BALANCE: ${balance.toFixed(2)}</p>
        </div>
      </div>

      {/* Balance Card */}
      <div style={{ padding: '0 32px 24px' }}>
        <div className="portfolio-balance-card">
          <div className="portfolio-balance-header">
            <span className="portfolio-balance-label">Total Balance</span>
            <span className="portfolio-balance-value">${balance.toFixed(2)}</span>
          </div>
          <div className="portfolio-balance-sub">
            {positions.length} {positions.length === 1 ? 'position' : 'positions'}
          </div>
        </div>
      </div>

      {/* Positions List */}
      <div style={{ padding: '0 32px 32px' }}>
        {positions.length === 0 ? (
          <div className="empty-state-card">
            <div className="empty-state-icon">📊</div>
            <div className="empty-state-title">No Active Positions</div>
            <div className="empty-state-subtitle">Start trading to see your positions here</div>
          </div>
        ) : (
          <div className="positions-grid">
            {Array.from(positionsByMarket.entries()).map(([marketId, marketPositions]) => {
              const firstPosition = marketPositions[0]
              const totalCost = marketPositions.reduce((sum, p) => sum + (p.amount * p.price), 0)
              const avgPrice = totalCost / marketPositions.reduce((sum, p) => sum + p.amount, 0)
              const totalAmount = marketPositions.reduce((sum, p) => sum + p.amount, 0)

              return (
                <div key={marketId} className="position-card">
                  <div className="position-header">
                    <div className="position-title-group">
                      <h3 className="position-title">{firstPosition.eventName}</h3>
                      <p className="position-subtitle">
                        {marketNames[firstPosition.market] || `Market ${firstPosition.market}`} • {marketId.substring(0, 16)}...
                      </p>
                    </div>
                    <div className={`position-badge ${firstPosition.side}`}>
                      {firstPosition.side.toUpperCase()}
                    </div>
                  </div>

                  <div className="position-details-grid">
                    <div className="position-detail-item">
                      <span className="position-detail-label">Total Amount</span>
                      <span className="position-detail-value">${totalAmount.toFixed(2)}</span>
                    </div>
                    <div className="position-detail-item">
                      <span className="position-detail-label">Avg Price</span>
                      <span className="position-detail-value">{(avgPrice * 100).toFixed(2)}¢</span>
                    </div>
                    <div className="position-detail-item">
                      <span className="position-detail-label">Total Cost</span>
                      <span className="position-detail-value">${totalCost.toFixed(2)}</span>
                    </div>
                    <div className="position-detail-item">
                      <span className="position-detail-label">Quantity</span>
                      <span className="position-detail-value">{marketPositions.length}</span>
                    </div>
                  </div>
                </div>
              )
            })}
          </div>
        )}
      </div>
    </>
  )
}

